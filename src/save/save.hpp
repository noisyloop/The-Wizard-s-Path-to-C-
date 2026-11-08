// Player state, save file, ranks and achievements.
#pragma once
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

struct Player {
    std::string name = "Apprentice";
    int xp = 0, gold = 0, hintScrolls = 1, potions = 1, bestStreak = 0;
    bool typewriter = true, sound = true;
    int dailyStreak = 0, lastDaily = 0;          // lastDaily = yyyymmdd
    std::set<std::string> cleared;               // realm ids whose boss is defeated
    std::vector<std::string> treasures;          // prizes + shop cosmetics
    std::set<std::string> achievements;
    std::map<std::string, int> stats;            // counters (spellsPassed, semicolonFixes, ...)
    bool owns(const std::string& t) const;
};

struct Rank { int xp; const char* title; const char* color; };
const std::vector<Rank>& ranks();
int rankIndex(int xp);

struct Achievement {
    std::string id, name, desc;
    std::function<bool(const Player&)> test;
};
const std::vector<Achievement>& achievementList();
// Awards any newly earned achievements (gold bonus). Returns their display lines.
std::vector<std::string> checkAchievements(Player& p, int totalRealms);

std::string savePath();
void save(const Player& p);
bool load(Player& p);

int todayStamp();            // yyyymmdd (local)
int yesterdayStamp();
