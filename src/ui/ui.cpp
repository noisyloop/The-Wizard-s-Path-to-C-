#include "ui.hpp"
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>
#ifdef _WIN32
#define NOMINMAX
#include <io.h>
#include <windows.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif
using namespace std;

namespace C {
const char* R = "\033[0m";   const char* B = "\033[1m";
const char* RED = "\033[91m"; const char* GRN = "\033[92m"; const char* YEL = "\033[93m";
const char* BLU = "\033[94m"; const char* MAG = "\033[95m"; const char* CYN = "\033[96m";
const char* GRY = "\033[90m";
}

namespace ui {
static bool g_typewriter = true, g_sound = true;

void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004 /*ENABLE_VIRTUAL_TERMINAL_PROCESSING*/);
    SetConsoleTitleA("Wizards++  -  The Wizard's Path to C++");
#endif
}

bool interactive() { return ISATTY(FILENO(stdin)) != 0; }
void clearScreen() { cout << "\033[2J\033[H"; }

string readLine() {
    string s;
    if (!getline(cin, s)) { cout << "\n" << C::MAG << "The tower doors close. Farewell, wizard." << C::R << "\n"; exit(0); }
    return s;
}

void pause() {
    cout << C::GRY << "\n  [ press ENTER to continue ]" << C::R;
    readLine();
}

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string norm(const string& s) {
    string out; bool sp = false;
    for (char ch : trim(s)) {
        if (isspace((unsigned char)ch)) { sp = true; continue; }
        if (sp && !out.empty()) out += ' ';
        sp = false;
        out += (char)tolower((unsigned char)ch);
    }
    return out;
}

string squash(const string& s) {
    string out;
    for (char ch : s) if (!isspace((unsigned char)ch)) out += (char)tolower((unsigned char)ch);
    return out;
}

void setTypewriter(bool on) { g_typewriter = on; }
void setSound(bool on) { g_sound = on; }

void say(const string& text) {
    if (!g_typewriter || !interactive()) { cout << text; return; }
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\033') {                       // pass escape sequences through whole
            size_t j = text.find('m', i);
            if (j == string::npos) j = i;
            cout << text.substr(i, j - i + 1); i = j; continue;
        }
        cout << text[i] << flush;
        if (!isspace((unsigned char)text[i])) this_thread::sleep_for(chrono::milliseconds(7));
    }
}

void beep(bool good) {
    if (!g_sound || !interactive()) return;
#ifdef _WIN32
    if (good) { Beep(880, 70); Beep(1175, 90); } else { Beep(220, 180); }
#else
    (void)good;
    cout << '\a' << flush;
#endif
}

void banner() {
    cout << C::MAG << C::B << R"WZ(
    #   #  ###  #####   ###   ####   ####    ####              
    #   #   #      #   #   #  #   #  #   #  #        #      #  
    # # #   #     #    #####  ####   #   #   ###    ###    ### 
    ## ##   #    #     #   #  #  #   #   #      #    #      #  
    #   #  ###  #####  #   #  #   #  ####   ####
)WZ" << C::R << C::YEL << "                  ~ The Wizard's Path to C++ ~\n" << C::R;
}

void wizardArt() {
    cout << C::BLU <<
R"(
                    *        .
              .         /\         *
                   *   /  \    .
                      /    \
          .          /______\        *
                    (  o  o  )
                     \  --  /     ~ <code>
                   ___\____/___  ~
                  /   | ++ |   \*
                 /    |    |    \
)" << C::R;
}

void showScroll(const string& code) {
    if (code.empty()) return;
    cout << C::YEL << "    .--------------------------------------------.\n" << C::R;
    stringstream ss(code); string line;
    while (getline(ss, line)) cout << C::YEL << "    | " << C::GRN << line << C::R << "\n";
    cout << C::YEL << "    '--------------------------------------------'\n" << C::R;
}
}
