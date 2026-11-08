// The game loop: menus, realms, trials, bosses, spells, practice, daily spell, shop.
#pragma once
#include "../content/content.hpp"
#include "../save/save.hpp"
#include <vector>

struct Game {
    Player p;
    std::vector<Realm> book;

    int run(const std::string& dataDir);   // returns process exit code

private:
    bool realmOpen(int idx) const;
    void mainMenu();
    void mapScreen();
    void playRealm(int idx, bool practice);
    void practiceMenu();
    void dailySpell();
    void bazaar();
    void chamber();
    void settings();
    void announce(const std::vector<std::string>& lines);
    void awardAchievements();
};
