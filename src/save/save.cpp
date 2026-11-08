#include "save.hpp"
#include "../ui/ui.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
using namespace std;

bool Player::owns(const string& t) const { return find(treasures.begin(), treasures.end(), t) != treasures.end(); }

const vector<Rank>& ranks() {
    static const vector<Rank> R = {
        {0,    "Hedge Apprentice", C::GRY}, {200,  "Rune Scribe",      C::GRN},
        {600,  "Adept Conjurer",   C::CYN}, {1400, "Sorcerer",         C::BLU},
        {2600, "Warlock of Loops", C::MAG}, {4200, "High Enchanter",   C::YEL},
        {6500, "ARCHMAGE OF C++",  C::RED},
    };
    return R;
}
int rankIndex(int xp) { int r = 0; for (size_t i = 0; i < ranks().size(); i++) if (xp >= ranks()[i].xp) r = (int)i; return r; }

static int stat(const Player& p, const string& k) { auto it = p.stats.find(k); return it == p.stats.end() ? 0 : it->second; }

const vector<Achievement>& achievementList() {
    static const vector<Achievement> A = {
        {"first_boss", "Tower Climber", "Defeat your first boss", [](const Player& p) { return p.cleared.size() >= 1; }},
        {"five_bosses", "Boss Slayer", "Defeat 5 bosses", [](const Player& p) { return p.cleared.size() >= 5; }},
        {"flawless", "Untouchable", "Beat a boss without losing mana", [](const Player& p) { return stat(p, "flawlessBosses") >= 1; }},
        {"streak5", "On Fire", "Reach a streak of 5", [](const Player& p) { return p.bestStreak >= 5; }},
        {"hundred", "Hundred Runes", "Answer 100 questions correctly", [](const Player& p) { return stat(p, "correct") >= 100; }},
        {"first_spell", "First Incantation", "Compile and pass your first real spell", [](const Player& p) { return stat(p, "spellsPassed") >= 1; }},
        {"ten_spells", "Compiler Whisperer", "Pass 10 spells", [](const Player& p) { return stat(p, "spellsPassed") >= 10; }},
        {"semicolons", "Semicolon Surgeon", "Fixed 10 missing semicolons", [](const Player& p) { return stat(p, "semicolonFixes") >= 10; }},
        {"daily3", "Daily Devotee", "Cast the Daily Spell 3 days in a row", [](const Player& p) { return p.dailyStreak >= 3; }},
        {"practice", "Practice Makes Perfect", "20 correct answers in Practice Mode", [](const Player& p) { return stat(p, "practiceCorrect") >= 20; }},
        {"collector", "Familiar Friend", "Own 3 Bazaar curiosities", [](const Player& p) { return stat(p, "bazaarBuys") >= 3; }},
    };
    return A;
}

vector<string> checkAchievements(Player& p, int totalRealms) {
    vector<string> lines;
    for (auto& a : achievementList()) {
        if (p.achievements.count(a.id) || !a.test(p)) continue;
        p.achievements.insert(a.id); p.gold += 25;
        lines.push_back("ACHIEVEMENT: " + a.name + " - " + a.desc + "  (+25 gold)");
    }
    if (totalRealms > 0 && (int)p.cleared.size() >= totalRealms && !p.achievements.count("archmage")) {
        p.achievements.insert("archmage"); p.gold += 100;
        lines.push_back("ACHIEVEMENT: Archmage - Every realm conquered  (+100 gold)");
    }
    return lines;
}

string savePath() { return "wizards_save.txt"; }

static string join(const vector<string>& v, char sep) { string o; for (size_t i = 0; i < v.size(); i++) { if (i) o += sep; o += v[i]; } return o; }
static vector<string> split(const string& s, char sep) {
    vector<string> out; string cur; stringstream ss(s);
    while (getline(ss, cur, sep)) if (!cur.empty()) out.push_back(cur);
    return out;
}

void save(const Player& p) {
    ofstream f(savePath());
    f << "name=" << p.name << "\nxp=" << p.xp << "\ngold=" << p.gold << "\nhints=" << p.hintScrolls
      << "\npotions=" << p.potions << "\nbestStreak=" << p.bestStreak << "\ntypewriter=" << p.typewriter
      << "\nsound=" << p.sound << "\ndailyStreak=" << p.dailyStreak << "\nlastDaily=" << p.lastDaily << "\n";
    f << "cleared=" << join(vector<string>(p.cleared.begin(), p.cleared.end()), ',') << "\n";
    f << "treasures=" << join(p.treasures, '|') << "\n";
    f << "achievements=" << join(vector<string>(p.achievements.begin(), p.achievements.end()), ',') << "\n";
    for (auto& kv : p.stats) f << "stat." << kv.first << "=" << kv.second << "\n";
}

bool load(Player& p) {
    ifstream f(savePath());
    if (!f) return false;
    string line;
    while (getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        string k = line.substr(0, eq), v = line.substr(eq + 1);
        auto num = [&]() { return atoi(v.c_str()); };
        if (k == "name") p.name = v.empty() ? p.name : v;
        else if (k == "xp") p.xp = num(); else if (k == "gold") p.gold = num();
        else if (k == "hints") p.hintScrolls = num(); else if (k == "potions") p.potions = num();
        else if (k == "bestStreak") p.bestStreak = num();
        else if (k == "typewriter") p.typewriter = num() != 0; else if (k == "sound") p.sound = num() != 0;
        else if (k == "dailyStreak") p.dailyStreak = num(); else if (k == "lastDaily") p.lastDaily = num();
        else if (k == "cleared") { for (auto& s : split(v, ',')) p.cleared.insert(s); }
        else if (k == "treasures") p.treasures = split(v, '|');
        else if (k == "achievements") { for (auto& s : split(v, ',')) p.achievements.insert(s); }
        else if (k.rfind("stat.", 0) == 0) p.stats[k.substr(5)] = num();
    }
    return true;
}

static int stampOf(time_t t) { tm lt = *localtime(&t); return (lt.tm_year + 1900) * 10000 + (lt.tm_mon + 1) * 100 + lt.tm_mday; }
int todayStamp() { return stampOf(time(nullptr)); }
int yesterdayStamp() {
    time_t now = time(nullptr); tm lt = *localtime(&now);
    lt.tm_mday -= 1; lt.tm_isdst = -1;
    return stampOf(mktime(&lt));
}
