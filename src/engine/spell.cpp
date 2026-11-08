#include "spell.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <set>
#ifdef _WIN32
#include <process.h>
#define POPEN _popen
#define PCLOSE _pclose
#define GETPID _getpid
#define NULLDEV "nul"
#else
#include <unistd.h>
#include <sys/wait.h>
#define POPEN popen
#define PCLOSE pclose
#define GETPID getpid
#define NULLDEV "/dev/null"
#endif
namespace fs = std::filesystem;
using namespace std;

static string runCapture(const string& cmd, int* status = nullptr) {
    string out;
    FILE* p = POPEN(cmd.c_str(), "r");
    if (!p) { if (status) *status = -1; return out; }
    char buf[512];
    while (size_t n = fread(buf, 1, sizeof buf, p)) { out.append(buf, n); if (out.size() > 200000) break; }
    int rc = PCLOSE(p);
    if (status) *status = rc;
    return out;
}

static string g_cxx;
static bool g_haveTimeout = false;

bool compilerAvailable() {
    static int cached = -1;
    if (cached >= 0) return cached == 1;
    for (const char* c : {"g++", "clang++", "c++"}) {
        int st = 1;
        runCapture(string(c) + " --version 2>" NULLDEV, &st);
        if (st == 0) { g_cxx = c; break; }
    }
#ifndef _WIN32
    int st = 1; runCapture("timeout 1 true 2>/dev/null", &st); g_haveTimeout = (st == 0);
#endif
    cached = g_cxx.empty() ? 0 : 1;
    return cached == 1;
}

static string trimEnd(string s) { while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back(); return s; }
static string normOut(const string& s) {
    stringstream in(s); string line, out;
    while (getline(in, line)) { line = trimEnd(line); out += line + "\n"; }
    return trimEnd(out);
}

static bool has(const string& s, const char* sub) { return s.find(sub) != string::npos; }

vector<string> translateErrors(const string& raw, bool* semicolon) {
    string low; for (char c : raw) low += (char)tolower((unsigned char)c);
    vector<string> h; set<string> seen;
    auto add = [&](const string& m) { if (seen.insert(m).second) h.push_back(m); };
    if (semicolon) *semicolon = false;
    if (has(low, "expected ';'") || has(low, "expected ‘;’")) {
        add("A semicolon is missing. Every statement ends with ';' - check the line just BEFORE the one the error names.");
        if (semicolon) *semicolon = true;
    }
    if (has(low, "'cout' is not a member of 'std'") || has(low, "no member named 'cout'") || has(low, "‘cout’ is not a member"))
        add("std::cout is unknown: summon <iostream> with  #include <iostream>  (the prelude may already do it).");
    if (has(low, "was not declared in this scope") || has(low, "use of undeclared identifier"))
        add("An unknown rune: a name is used before it is declared. Check the spelling and capital letters.");
    if (has(low, "expected '}'") || has(low, "expected ‘}’") || has(low, "expected '}' at end of input"))
        add("A closing brace '}' is missing - every '{' needs a matching '}'.");
    if (has(low, "expected ')'") || has(low, "expected ‘)’"))
        add("A closing parenthesis ')' is missing.");
    if (has(low, "missing terminating") || has(low, "unterminated"))
        add("A string or character is missing its closing quote.");
    if (has(low, "expected primary-expression") || has(low, "expected expression"))
        add("The compiler wanted a value or expression and found something else - look for a stray operator or an unfinished line.");
    if (has(low, "does not name a type") || has(low, "unknown type name"))
        add("An unknown type: check spelling, capital letters, and #includes (string and vector live in headers!).");
    if (has(low, "no match for 'operator") || has(low, "invalid operands"))
        add("That operator can't combine those types (for example, a string and a number without conversion).");
    if (has(low, "invalid conversion") || has(low, "cannot convert") || has(low, "no viable conversion"))
        add("A value of the wrong type was used. Check what the spell's types should be.");
    if (has(low, "too few arguments") || has(low, "too many arguments") || has(low, "no matching function"))
        add("The function call doesn't match the function's parameters - count them and check the types.");
    if (has(low, "undefined reference") || has(low, "undefined symbols") || has(low, "ld returned"))
        add("Linker complaint: something was DECLARED but never DEFINED. Did you write the function/member bodies?");
    if (has(low, "redefinition") || has(low, "has already been defined"))
        add("Something was defined twice - remove the duplicate.");
    if (has(low, "assignment of read-only") || (has(low, "cannot assign to variable") && has(low, "const")))
        add("You tried to change a const. const things are sealed!");
    if (has(low, "no return statement") || has(low, "non-void function does not return"))
        add("This function promises to return a value but may end without one.");
    if (has(low, "private within this context") || has(low, "is a private member"))
        add("That member is private. Put it under public: or use a public function.");
    if (has(low, "abstract") && has(low, "cannot"))
        add("You can't create an object of an abstract class - implement every pure virtual function.");
    if (h.empty() && has(low, "error")) add("The compiler is displeased. Read the first error below - it's usually the real culprit.");
    return h;
}

static string scrub(string s, const string& dir) {
    for (size_t pos; (pos = s.find(dir + "/")) != string::npos;) s.erase(pos, dir.size() + 1);
    for (size_t pos; (pos = s.find(dir + "\\")) != string::npos;) s.erase(pos, dir.size() + 1);
    for (size_t pos; (pos = s.find(dir)) != string::npos;) s.erase(pos, dir.size());
    return s;
}

static string firstErrors(const string& s, int maxLines = 12) {
    stringstream in(s); string line, out; int n = 0;
    while (getline(in, line) && n < maxLines) { out += "    " + line + "\n"; n++; }
    return out;
}

static string q(const fs::path& p) { return "\"" + p.string() + "\""; }

SpellResult castSpell(const Spell& spell, const vector<string>& userParts) {
    SpellResult r;
    if (!compilerAvailable()) { r.note = "No C++ compiler found."; return r; }
    static int counter = 0;
    error_code ec;
    fs::path dir = fs::temp_directory_path(ec) / ("wizards_spell_" + to_string(GETPID()) + "_" + to_string(counter++));
    fs::create_directories(dir, ec);
    vector<string> sources;
    for (size_t i = 0; i < spell.files.size(); i++) {
        const SpellFile& f = spell.files[i];
        string body = f.prelude + (i < userParts.size() ? userParts[i] : "") + "\n" + f.epilogue;
        ofstream(dir / f.name) << body;
        if (fs::path(f.name).extension() == ".cpp") sources.push_back(f.name);
    }
    if (!spell.mainSrc.empty()) { ofstream(dir / "main.cpp") << spell.mainSrc; sources.push_back("main.cpp"); }
    string exe = (dir / (
#ifdef _WIN32
        "spell.exe"
#else
        "spell"
#endif
        )).string();

    string cmd = g_cxx + " -std=c++17 -O0 -w -I" + q(dir) + " -o " + q(exe);
    for (auto& s : sources) cmd += " " + q(dir / s);
    cmd += " 2>&1";
    int st = 0;
    string cout_ = runCapture(cmd, &st);
    if (st != 0 || !fs::exists(exe)) {
        r.compiled = false;
        r.rawErrors = firstErrors(scrub(cout_, dir.string()));
        r.wizardHints = translateErrors(cout_, &r.semicolonFix);
        fs::remove_all(dir, ec);
        return r;
    }
    r.compiled = true;
    string run = q(exe);
#ifndef _WIN32
    if (g_haveTimeout) run = "timeout 5 " + run;
#endif
    int rst = 0;
    string out = runCapture(run + " 2>&1 < " NULLDEV, &rst);
    fs::remove_all(dir, ec);
    r.output = normOut(out);
    
#ifndef _WIN32
    if (g_haveTimeout && rst != 0 && WIFEXITED(rst) && WEXITSTATUS(rst) == 124)
#else
    if (false)
#endif
        r.note = "Your spell ran too long (5s limit) - infinite loop?";
    r.passed = (r.output == normOut(spell.expected)) && r.note.empty();
    return r;
}
