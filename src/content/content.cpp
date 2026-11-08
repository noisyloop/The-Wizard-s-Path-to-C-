#include "content.hpp"
#include "../engine/json.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
using namespace std;

static bool hasContent(const fs::path& p) {
    error_code ec;
    return fs::is_directory(p / "content", ec);
}

string findDataDir(const char* argv0) {
    vector<fs::path> tries;
    if (const char* h = getenv("WIZARDS_HOME")) tries.push_back(h);
    error_code ec;
    if (argv0 && *argv0) {
        fs::path exe = fs::absolute(argv0, ec).parent_path();
        tries.push_back(exe);
        tries.push_back(exe / "..");
        tries.push_back(exe / ".." / "..");
    }
    tries.push_back(fs::current_path(ec));
    tries.push_back(fs::current_path(ec) / "..");
    for (auto& p : tries) if (hasContent(p)) return fs::weakly_canonical(p, ec).string();
    return ".";
}

static Question parseQuestion(const Json& j) {
    Question q;
    q.type = j["type"].str() == "type" ? QType::Type : QType::Choice;
    q.prompt = j["prompt"].str(); q.code = j["code"].str();
    q.options = j["options"].strs(); q.answers = j["answers"].strs();
    q.hint = j["hint"].str("Think it through."); q.explain = j["explain"].str();
    return q;
}

static bool questionOk(const Question& q) {
    if (q.prompt.empty() || q.answers.empty()) return false;
    if (q.type == QType::Choice && (q.options.empty() || q.answers[0].empty())) return false;
    return true;
}

static vector<Question> parseQuestions(const Json& j, const string& where, vector<string>& warn) {
    vector<Question> out;
    for (size_t i = 0; i < j.a.size(); i++) {
        Question q = parseQuestion(j.a[i]);
        if (questionOk(q)) out.push_back(q);
        else warn.push_back(where + ": skipped malformed question #" + to_string(i + 1));
    }
    return out;
}

static bool parseRealm(const Json& j, Realm& r, const string& where, vector<string>& warn) {
    r.id = j["id"].str(); r.name = j["name"].str(); r.realm = j["realm"].str();
    r.lesson = j["lesson"].str(); r.lessonCode = j["lessonCode"].str();
    r.bossName = j["bossName"].str("The Guardian"); r.bossArt = j["bossArt"].str(); r.prize = j["prize"].str("A Mysterious Trinket");
    r.trial = parseQuestions(j["trial"], where, warn);
    r.pool = parseQuestions(j["pool"], where, warn);
    r.boss = parseQuestions(j["boss"], where, warn);
    if (r.id.empty()) r.id = fs::path(where).stem().string();
    if (r.name.empty() || r.trial.size() + r.pool.size() < 3 || r.boss.empty()) {
        warn.push_back(where + ": realm needs a name, >=3 trial questions (trial+pool) and >=1 boss question");
        return false;
    }
    const Json& s = j["spell"];
    if (s.t == Json::Obj) {
        r.spell.prompt = s["prompt"].str(); r.spell.expected = s["expected"].str();
        r.spell.hint = s["hint"].str("Read the compiler's message carefully.");
        r.spell.explain = s["explain"].str();
        r.spell.mainSrc = s["mainSrc"].str();
        if (s["files"].t == Json::Arr) {
            for (auto& f : s["files"].a)
                r.spell.files.push_back({f["name"].str("main.cpp"), f["prelude"].str(), f["epilogue"].str()});
        } else {
            r.spell.files.push_back({"main.cpp", s["prelude"].str(), s["epilogue"].str()});
        }
    }
    return true;
}

static void loadDir(const fs::path& dir, bool core, vector<Realm>& out, vector<string>& warn) {
    error_code ec;
    if (!fs::is_directory(dir, ec)) return;
    vector<fs::path> files;
    for (auto& e : fs::directory_iterator(dir, ec))
        if (e.is_regular_file() && e.path().extension() == ".json") files.push_back(e.path());
    sort(files.begin(), files.end());
    for (auto& f : files) {
        ifstream in(f, ios::binary);
        stringstream ss; ss << in.rdbuf();
        string text = ss.str(), err;
        Json j;
        if (!JsonParser(text).parse(j, err)) { warn.push_back(f.filename().string() + ": " + err); continue; }
        Realm r;
        if (!parseRealm(j, r, f.filename().string(), warn)) continue;
        bool dup = false;
        for (auto& o : out) if (o.id == r.id) dup = true;
        if (dup) { warn.push_back(f.filename().string() + ": duplicate realm id '" + r.id + "' ignored"); continue; }
        r.core = core; r.source = f.string();
        out.push_back(std::move(r));
    }
}

vector<Realm> loadRealms(const string& dataDir, vector<string>& warnings) {
    vector<Realm> realms;
    loadDir(fs::path(dataDir) / "content", true, realms, warnings);
    loadDir(fs::path(dataDir) / "packs", false, realms, warnings);
    return realms;
}
