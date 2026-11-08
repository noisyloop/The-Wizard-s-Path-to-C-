#include "game.hpp"
#include "../ui/ui.hpp"
#include "../world/world.hpp"
#include "spell.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
using namespace std;
using ui::readLine; using ui::pause; using ui::clearScreen; using ui::trim; using ui::norm; using ui::squash;

static mt19937& rng() { static mt19937 r((unsigned)random_device{}()); return r; }

// ---------------------------------------------------------------- HUD
static void hud(const Player& p, int mana = -1, int streak = -1) {
    int r = rankIndex(p.xp);
    const auto& R = ranks();
    cout << C::GRY << "  -----------------------------------------------------------------\n" << C::R;
    cout << "  " << C::B << p.name << C::R << "  |  " << R[r].color << R[r].title << C::R
         << "  |  " << C::CYN << p.xp << " XP" << C::R << "  |  " << C::YEL << p.gold << " gold" << C::R;
    if (mana >= 0) { cout << "  |  " << C::BLU << "Mana "; for (int i = 0; i < mana; i++) cout << "*"; cout << C::R; }
    if (streak > 1) cout << "  |  " << C::RED << "Streak x" << streak << C::R;
    cout << "\n";
    if (r + 1 < (int)R.size()) {
        int lo = R[r].xp, hi = R[r + 1].xp;
        int filled = (p.xp - lo) * 30 / (hi - lo);
        cout << "  Next rank [" << C::CYN;
        for (int i = 0; i < 30; i++) cout << (i < filled ? '#' : '.');
        cout << C::R << "] " << (hi - p.xp) << " XP to " << R[r + 1].title << "\n";
    }
    cout << C::GRY << "  -----------------------------------------------------------------\n" << C::R;
}

static void levelUpCheck(int before, int after) {
    int a = rankIndex(before), b = rankIndex(after);
    if (b > a) {
        cout << "\n" << C::YEL << C::B
             << "  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *\n"
             << "     RANK UP!  You are now: " << ranks()[b].title << "\n"
             << "  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *" << C::R << "\n";
        ui::beep(true);
    }
}

void Game::announce(const vector<string>& lines) {
    for (auto& l : lines) { cout << "  " << C::YEL << C::B << "* " << l << C::R << "\n"; ui::beep(true); }
}
void Game::awardAchievements() { announce(checkAchievements(p, (int)book.size())); }

// ---------------------------------------------------------------- Questions
// Returns 1 correct, 0 wrong. Handles hints and potions. In practice mode a wrong answer costs nothing.
static int ask(Player& p, const Question& q, int num, int total, int& mana, int streak, bool bossMode, bool practice) {
    bool hinted = false;
    while (true) {
        cout << "\n  " << (bossMode ? C::RED : C::CYN) << C::B
             << (bossMode ? "BOSS ATTACK " : practice ? "PRACTICE " : "TRIAL ") << num << "/" << total << C::R << "\n";
        cout << "  " << C::B << q.prompt << C::R << "\n";
        ui::showScroll(q.code);
        for (auto& o : q.options) cout << "      " << o << "\n";
        if (hinted) cout << "  " << C::MAG << "Hint: " << q.hint << C::R << "\n";
        cout << C::GRY << "  (type 'hint' to use a Hint Scroll [" << p.hintScrolls << "], 'potion' to drink a Mana Potion ["
             << p.potions << "])\n" << C::R << "  " << C::YEL << "> " << C::R;
        string in = readLine(), n = norm(in);

        if (n == "hint") {
            if (hinted) { cout << "  You already read the hint.\n"; continue; }
            if (p.hintScrolls <= 0) { cout << "  " << C::RED << "No Hint Scrolls! Buy some in the Bazaar." << C::R << "\n"; continue; }
            p.hintScrolls--; hinted = true; continue;
        }
        if (n == "potion") {
            if (p.potions <= 0) { cout << "  " << C::RED << "No potions left." << C::R << "\n"; continue; }
            p.potions--; mana++; cout << "  " << C::BLU << "Glug glug... +1 Mana (" << mana << ")" << C::R << "\n"; continue;
        }
        if (n.empty()) continue;

        bool ok = false;
        if (q.type == QType::Choice) {
            ok = n[0] == q.answers[0][0] && (n.size() == 1 || n[1] == ')' || n[1] == ' ');
        } else {
            for (auto& a : q.answers) if (norm(a) == n || squash(a) == squash(in)) ok = true;
        }
        if (ok) {
            static const char* cheers[] = {"The runes glow!", "Spell cast flawlessly!", "Sparks fly from your wand!",
                                           "The ancient tome approves.", "Brilliant sorcery!", "Your familiar hoots in delight!"};
            cout << "  " << C::GRN << C::B << "+ " << cheers[(num + streak) % 6] << C::R << "\n";
            ui::beep(true);
        } else {
            if (!practice) mana--;
            cout << "  " << C::RED << C::B << "x The spell backfires!" << (practice ? "" : " -1 Mana") << C::R << "\n";
            cout << "  Correct answer: " << C::GRN << q.answers[0] << C::R << "\n";
            ui::beep(false);
        }
        if (!q.explain.empty()) cout << "  " << C::GRY << "Lore: " << q.explain << C::R << "\n";
        return ok ? 1 : 0;
    }
}

// Runs a set of questions in the given order. Returns true if survived. manaLost reports mana spent.
static bool runSet(Player& p, const vector<Question>& qs, int realmIdx, bool boss, bool practice, int& manaLost, vector<string>& earned) {
    int mana = 3, streak = 0;
    int baseXP = 10 + realmIdx * 5; if (boss) baseXP *= 2;
    int goldEach = practice ? 3 : 5 + realmIdx * 2;
    for (size_t i = 0; i < qs.size(); i++) {
        hud(p, mana, streak);
        int before = p.xp;
        if (ask(p, qs[i], (int)i + 1, (int)qs.size(), mana, streak, boss, practice)) {
            streak++;
            p.bestStreak = max(p.bestStreak, streak);
            p.stats["correct"]++;
            if (practice) p.stats["practiceCorrect"]++;
            int bonus = (!practice && streak >= 3) ? baseXP / 2 : 0;
            if (!practice) p.xp += baseXP + bonus;
            p.gold += goldEach;
            if (!practice) cout << "  " << C::CYN << "+" << baseXP << " XP" << C::R;
            if (bonus) cout << C::RED << "  +" << bonus << " streak bonus!" << C::R;
            cout << C::YEL << "  +" << goldEach << " gold" << C::R << "\n";
            levelUpCheck(before, p.xp);
        } else streak = 0;
        if (mana <= 0) { manaLost = 3 - max(mana, 0); cout << "\n  " << C::RED << C::B << "Your mana is spent. You collapse..." << C::R << "\n"; return false; }
        pause(); clearScreen();
    }
    manaLost = 3 - mana;
    (void)earned;
    return true;
}

// ---------------------------------------------------------------- Spell casting
static string readEditor(const SpellFile& f, bool& skipped, const string& hint) {
    cout << "  " << C::CYN << "Write the missing code for " << C::B << f.name << C::R << C::CYN
         << ".  Type lines, then " << C::B << "/done" << C::R << C::CYN << " (or /hint, /skip)." << C::R << "\n";
    string code;
    while (true) {
        cout << C::GRY << "  | " << C::R;
        string line = readLine();
        string t = trim(line);
        if (t == "/done") break;
        if (t == "/skip") { skipped = true; return ""; }
        if (t == "/hint") { cout << "  " << C::MAG << "Hint: " << hint << C::R << "\n"; continue; }
        code += line + "\n";
    }
    return code;
}

static void showSpellScaffold(const Spell& s) {
    for (auto& f : s.files) {
        cout << C::GRY << "  --- " << f.name << " ---" << C::R << "\n";
        if (!f.prelude.empty()) ui::showScroll(f.prelude);
        cout << C::MAG << "      <<< your spell goes here >>>" << C::R << "\n";
        if (!f.epilogue.empty()) ui::showScroll(f.epilogue);
    }
    if (!s.mainSrc.empty()) { cout << C::GRY << "  --- main.cpp (given) ---" << C::R << "\n"; ui::showScroll(s.mainSrc); }
}

// Returns true if the spell passed within 3 attempts.
static bool castSpellChallenge(Player& p, const Spell& s, int realmIdx, bool practice, const string& title) {
    cout << "\n  " << C::MAG << C::B << "~ SPELL CASTING: " << title << " ~" << C::R << "\n";
    cout << "  " << C::B << s.prompt << C::R << "\n\n";
    showSpellScaffold(s);
    cout << "\n  Expected output:\n";
    ui::showScroll(s.expected);
    bool hintShown = false;
    for (int attempt = 1; attempt <= 3; attempt++) {
        cout << "\n  " << C::YEL << "Attempt " << attempt << "/3" << C::R << "\n";
        vector<string> parts; bool skipped = false;
        for (auto& f : s.files) {
            string code = readEditor(f, skipped, s.hint);
            if (skipped) break;
            parts.push_back(code);
        }
        if (skipped) { cout << "  You lower your wand. The spell fizzles.\n"; return false; }
        cout << C::GRY << "  Casting..." << C::R << "\n";
        SpellResult r = castSpell(s, parts);
        if (!r.compiled) {
            cout << "  " << C::RED << C::B << "x The spell failed to compile!" << C::R << "\n";
            for (auto& h : r.wizardHints) cout << "  " << C::MAG << "Wizard hint: " << h << C::R << "\n";
            if (!r.rawErrors.empty()) cout << C::GRY << "  Compiler said:\n" << r.rawErrors << C::R;
            if (r.semicolonFix) p.stats["semicolonFixes"]++;
            ui::beep(false);
        } else if (!r.passed) {
            cout << "  " << C::RED << C::B << "x It compiled, but the output is wrong." << C::R << "\n";
            if (!r.note.empty()) cout << "  " << C::MAG << r.note << C::R << "\n";
            cout << "  Expected:\n"; ui::showScroll(s.expected);
            cout << "  You got:\n"; ui::showScroll(r.output.empty() ? "(nothing)" : r.output);
            ui::beep(false);
        } else {
            cout << "  " << C::GRN << C::B << "+ The spell works perfectly!" << C::R << "\n";
            if (!s.explain.empty()) cout << "  " << C::GRY << "Lore: " << s.explain << C::R << "\n";
            int xp = (practice ? 0 : (10 + realmIdx * 5) * 3), gold = practice ? 8 : 15 + realmIdx * 3;
            int before = p.xp;
            p.xp += xp; p.gold += gold; p.stats["spellsPassed"]++;
            cout << "  " << C::CYN << (xp ? "+" + to_string(xp) + " XP  " : "") << C::YEL << "+" << gold << " gold" << C::R << "\n";
            levelUpCheck(before, p.xp);
            ui::beep(true);
            return true;
        }
        if (attempt < 3 && !hintShown) {
            cout << "  " << C::GRY << "Hint: " << s.hint << C::R << "\n"; hintShown = true;
        }
    }
    cout << "  " << C::RED << "Out of attempts. Reference solution hint: " << s.hint << C::R << "\n";
    return false;
}

// ---------------------------------------------------------------- Realm
bool Game::realmOpen(int idx) const { return idx == 0 || p.cleared.count(book[idx - 1].id); }

static vector<Question> pickTrial(const Realm& r) {
    vector<Question> all = r.trial;
    all.insert(all.end(), r.pool.begin(), r.pool.end());
    shuffle(all.begin(), all.end(), rng());
    // keep a mix: prefer to include at least one of each type when available
    all.resize(min<size_t>(3, all.size()));
    return all;
}

void Game::playRealm(int idx, bool practice) {
    const Realm& c = book[idx];
    clearScreen();
    cout << C::MAG << C::B << "\n  ~~~ Realm " << idx + 1 << ": " << c.name << (practice ? "  (PRACTICE)" : "") << " ~~~" << C::R << "\n";
    cout << "  " << C::GRY << c.realm << C::R << "\n\n";
    cout << C::CYN << C::B << "  THE MASTER'S LESSON" << C::R << "\n  ";
    ui::say(c.lesson); cout << "\n\n";
    ui::showScroll(c.lessonCode);
    pause(); clearScreen();

    int lost = 0; vector<string> earned;
    cout << C::CYN << C::B << "\n  ~ The Trials of " << c.name << " ~" << C::R << "\n";
    if (!runSet(p, pickTrial(c), idx, false, practice, lost, earned)) {
        cout << "  Study the lesson and return stronger.\n"; save(p); pause(); return;
    }
    clearScreen();
    cout << C::RED << C::B << "\n  A shadow falls across the chamber...\n\n" << C::R;
    cout << C::RED << c.bossArt << C::R << "\n\n";
    cout << "  " << C::B << c.bossName << C::R << " blocks your path!\n";
    cout << "  " << C::GRY << (practice ? "(Practice: mistakes cost no mana, no XP.)" : "(Boss battles give double XP. You have 3 Mana.)") << C::R << "\n";
    pause(); clearScreen();

    vector<Question> bossQs = c.boss; shuffle(bossQs.begin(), bossQs.end(), rng());
    if (!runSet(p, bossQs, idx, true, practice, lost, earned)) {
        cout << "  " << c.bossName << " laughs as you retreat. Try again!\n"; save(p); pause(); return;
    }

    // Final spell: the boss becomes a coding puzzle.
    if (c.spell.valid()) {
        clearScreen();
        cout << C::RED << C::B << "\n  " << c.bossName << " raises a final challenge: write a REAL spell!" << C::R << "\n";
        if (!compilerAvailable()) {
            cout << "  " << C::YEL << "(No C++ compiler found on this machine - the spell trial is waived.\n"
                 << "   Install g++ / MinGW to unlock Spell Casting.)" << C::R << "\n";
            pause();
        } else if (!castSpellChallenge(p, c.spell, idx, practice, c.bossName)) {
            cout << "  " << c.bossName << " cackles. Your spell wasn't ready. Try again!\n"; save(p); pause(); return;
        } else pause();
    }

    clearScreen();
    bool first = !p.cleared.count(c.id);
    cout << C::YEL << C::B << "\n  ****************************************************\n"
         << "     VICTORY!  " << c.bossName << " is defeated!\n"
         << "  ****************************************************" << C::R << "\n\n";
    if (practice) {
        cout << "  Practice complete. Gold earned, no risk taken.\n";
    } else if (first) {
        p.cleared.insert(c.id); p.treasures.push_back(c.prize);
        if (lost == 0) { p.stats["flawlessBosses"]++; cout << "  " << C::GRN << "Flawless victory - not a drop of mana lost!" << C::R << "\n"; }
        int before = p.xp; p.xp += 50; p.gold += 40;
        cout << "  " << C::MAG << "You found: " << C::B << c.prize << C::R << "\n";
        cout << "  " << C::CYN << "+50 XP" << C::R << "   " << C::YEL << "+40 gold" << C::R << "\n";
        levelUpCheck(before, p.xp);
        if (idx + 1 < (int)book.size()) cout << "\n  " << C::GRN << "A new realm opens: " << book[idx + 1].name << C::R << "\n";
        if ((int)p.cleared.size() >= (int)book.size())
            cout << "\n" << C::YEL << C::B << "  You have conquered every realm of Wizards++.\n"
                 << "  The Council of Code bows before you, ARCHMAGE.\n"
                 << "  (Replay realms in Practice Mode, cast the Daily Spell, or write your own realm pack!)" << C::R << "\n";
    } else {
        cout << "  You've beaten this boss before. (Use Practice Mode to replay without risk.)\n";
    }
    awardAchievements();
    save(p); pause();
}

void Game::mapScreen() {
    while (true) {
        clearScreen();
        cout << C::CYN << C::B << "\n  ~ The Map of Realms ~" << C::R << "\n\n";
        hud(p);
        for (size_t i = 0; i < book.size(); i++) {
            bool open = realmOpen((int)i), done = p.cleared.count(book[i].id);
            cout << "   " << i + 1 << ") ";
            if (done) cout << C::GRN << "[*] "; else if (open) cout << C::YEL << "[ ] "; else cout << C::GRY << "[locked] ";
            cout << book[i].name << C::R;
            if (!book[i].core) cout << C::MAG << "  (pack)" << C::R;
            if (open && !done) cout << C::YEL << "   <- your next quest" << C::R;
            cout << "\n";
            if (i + 1 < book.size()) cout << C::GRY << "       |" << C::R << "\n";
        }
        cout << "\n   0) Back\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "0" || s.empty()) return;
        int k = atoi(s.c_str()) - 1;
        if (k < 0 || k >= (int)book.size()) continue;
        if (!realmOpen(k)) { cout << "  " << C::RED << "A magic barrier blocks the way. Defeat the previous boss first." << C::R << "\n"; pause(); continue; }
        playRealm(k, false);
    }
}

void Game::practiceMenu() {
    while (true) {
        clearScreen();
        cout << C::GRN << C::B << "\n  ~ Practice Grounds ~" << C::R << "\n"
             << C::GRY << "  Replay cleared realms for gold. No mana loss, no XP, no risk.\n" << C::R << "\n";
        hud(p);
        vector<int> idxs;
        for (size_t i = 0; i < book.size(); i++) if (p.cleared.count(book[i].id)) { idxs.push_back((int)i); cout << "   " << idxs.size() << ") " << book[i].name << "\n"; }
        if (idxs.empty()) { cout << "   (Clear a realm first!)\n"; }
        cout << "\n   0) Back\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "0" || s.empty()) return;
        int k = atoi(s.c_str()) - 1;
        if (k < 0 || k >= (int)idxs.size()) continue;
        playRealm(idxs[k], true);
    }
}

// ---------------------------------------------------------------- Daily spell
void Game::dailySpell() {
    clearScreen();
    cout << C::YEL << C::B << "\n  ~ The Daily Spell ~" << C::R << "\n";
    hud(p);
    int today = todayStamp();
    cout << "  Streak: " << C::RED << p.dailyStreak << " day(s)" << C::R << "\n";
    if (p.lastDaily == today) { cout << "\n  You already cast today's spell. Come back tomorrow!\n"; pause(); return; }
    if (!compilerAvailable()) { cout << "\n  The Daily Spell needs a C++ compiler (g++ / MinGW) on this machine.\n"; pause(); return; }
    vector<int> pool;
    for (size_t i = 0; i < book.size(); i++) if (book[i].spell.valid() && (p.cleared.count(book[i].id) || realmOpen((int)i))) pool.push_back((int)i);
    if (pool.empty()) { cout << "  No spells available yet.\n"; pause(); return; }
    int idx = pool[(size_t)(today % 100000) % pool.size()];
    cout << "\n  Today's spell is drawn from: " << C::B << book[idx].name << C::R << "\n";
    if (!castSpellChallenge(p, book[idx].spell, idx, false, "Daily Spell")) { save(p); pause(); return; }
    p.dailyStreak = (p.lastDaily == yesterdayStamp()) ? p.dailyStreak + 1 : 1;
    p.lastDaily = today;
    int bonus = 20 + p.dailyStreak * 5;
    p.gold += bonus;
    cout << "  " << C::YEL << "Daily bonus: +" << bonus << " gold!  Streak: " << p.dailyStreak << C::R << "\n";
    awardAchievements(); save(p); pause();
}

// ---------------------------------------------------------------- Bazaar / chamber / settings
void Game::bazaar() {
    struct Item { string name; int cost; int kind; };  // 0 hint, 1 potion, 2 cosmetic
    vector<Item> items = {
        {"Hint Scroll", 15, 0}, {"Mana Potion", 25, 1}, {"Owl Familiar 'Semicolon'", 60, 2},
        {"Hat of Infinite Recursion", 90, 2}, {"Cloak of Compile-Time Safety", 120, 2}, {"Phoenix Familiar 'Segfault'", 200, 2},
    };
    while (true) {
        clearScreen();
        cout << C::YEL << C::B << "\n  ~ The Midnight Bazaar ~" << C::R << "\n";
        cout << C::GRY << "  A hooded merchant grins. \"Gold for magic, wizard?\"" << C::R << "\n\n";
        hud(p);
        for (size_t i = 0; i < items.size(); i++) {
            bool owned = items[i].kind == 2 && p.owns(items[i].name);
            cout << "   " << i + 1 << ") " << items[i].name << "  " << C::YEL << items[i].cost << "g" << C::R
                 << (owned ? string(C::GRN) + "  (owned)" + C::R : "") << "\n";
        }
        cout << "   0) Leave\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "0" || s.empty()) return;
        int k = atoi(s.c_str()) - 1;
        if (k < 0 || k >= (int)items.size()) continue;
        Item& it = items[k];
        if (it.kind == 2 && p.owns(it.name)) { cout << "  You already own that.\n"; pause(); continue; }
        if (p.gold < it.cost) { cout << "  " << C::RED << "Not enough gold." << C::R << "\n"; pause(); continue; }
        p.gold -= it.cost;
        if (it.kind == 0) p.hintScrolls++; else if (it.kind == 1) p.potions++;
        else { p.treasures.push_back(it.name); p.stats["bazaarBuys"]++; }
        cout << "  " << C::GRN << "Purchased " << it.name << "!" << C::R << "\n";
        awardAchievements(); save(p); pause();
    }
}

void Game::chamber() {
    clearScreen();
    cout << C::MAG << C::B << "\n  ~ Your Wizard's Chamber ~" << C::R << "\n\n";
    hud(p);
    cout << "  Realms conquered: " << p.cleared.size() << "/" << book.size() << "\n";
    cout << "  Best streak:      " << p.bestStreak << "      Spells cast: " << p.stats["spellsPassed"] << "\n";
    cout << "  Hint Scrolls:     " << p.hintScrolls << "     Mana Potions: " << p.potions << "     Daily streak: " << p.dailyStreak << "\n\n";
    cout << C::YEL << "  TREASURES" << C::R << "\n";
    if (p.treasures.empty()) cout << "   (empty - defeat bosses to earn wizard prizes!)\n";
    for (auto& t : p.treasures) cout << "   * " << t << "\n";
    cout << "\n" << C::YEL << "  RANKS" << C::R << "\n";
    for (auto& r : ranks())
        cout << "   " << (p.xp >= r.xp ? C::GRN : C::GRY) << (p.xp >= r.xp ? "[x] " : "[ ] ") << r.title << "  (" << r.xp << " XP)" << C::R << "\n";
    cout << "\n" << C::YEL << "  ACHIEVEMENTS" << C::R << "\n";
    for (auto& a : achievementList()) {
        bool got = p.achievements.count(a.id);
        cout << "   " << (got ? C::GRN : C::GRY) << (got ? "[x] " : "[ ] ") << a.name << " - " << a.desc << C::R << "\n";
    }
    cout << "   " << (p.achievements.count("archmage") ? C::GRN : C::GRY) << (p.achievements.count("archmage") ? "[x] " : "[ ] ")
         << "Archmage - Every realm conquered" << C::R << "\n";
    pause();
}

void Game::settings() {
    while (true) {
        clearScreen();
        cout << C::CYN << C::B << "\n  ~ Settings ~" << C::R << "\n\n";
        cout << "   1) Typewriter text: " << (p.typewriter ? "ON" : "OFF") << "\n";
        cout << "   2) Sound beeps:     " << (p.sound ? "ON" : "OFF") << "\n";
        cout << "   3) Spell compiler:  " << (compilerAvailable() ? "found" : "NOT FOUND (spells disabled)") << "\n";
        cout << "   0) Back\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "1") p.typewriter = !p.typewriter; else if (s == "2") p.sound = !p.sound; else if (s == "0" || s.empty()) { save(p); return; }
        ui::setTypewriter(p.typewriter); ui::setSound(p.sound);
    }
}

// ---------------------------------------------------------------- Menu
void Game::mainMenu() {
    while (true) {
        clearScreen();
        ui::banner();
        hud(p);
        vector<pair<string, string>> items = {
            {"journey", string(C::CYN) + "Journey" + C::R + "          - continue your quest"},
            {"practice", string(C::GRN) + "Practice Mode" + C::R + "     - replay cleared realms for gold, risk-free"},
            {"daily", string(C::YEL) + "Daily Spell" + C::R + "        - one new coding challenge a day"},
            {"bazaar", string(C::YEL) + "Midnight Bazaar" + C::R + "   - hints, potions, familiars"},
            {"chamber", string(C::MAG) + "Wizard's Chamber" + C::R + "  - treasures, ranks, achievements"},
            {"settings", string(C::GRY) + "Settings" + C::R},
        };
        if (worldAvailable()) items.insert(items.begin() + 1, {"world", string(C::BLU) + "Walk the World" + C::R + "    - explore the 2D map"});
        items.push_back({"quit", string(C::GRY) + "Leave the tower" + C::R});
        cout << "\n";
        for (size_t i = 0; i < items.size(); i++) cout << "   " << i + 1 << ") " << items[i].second << "\n";
        cout << "\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        int k = atoi(s.c_str()) - 1;
        if (k < 0 || k >= (int)items.size()) continue;
        const string& id = items[k].first;
        if (id == "journey") mapScreen();
        else if (id == "practice") practiceMenu();
        else if (id == "daily") dailySpell();
        else if (id == "bazaar") bazaar();
        else if (id == "chamber") chamber();
        else if (id == "settings") settings();
        else if (id == "world") {
            while (true) {
                vector<bool> open, done;
                for (size_t i = 0; i < book.size(); i++) { open.push_back(realmOpen((int)i)); done.push_back(p.cleared.count(book[i].id) > 0); }
                vector<string> names; for (auto& r : book) names.push_back(r.name);
                vector<string> bosses; for (auto& r : book) bosses.push_back(r.bossName);
                int pick = worldPick(names, bosses, open, done, p.treasures, p.name);
                if (pick < 0) break;
                playRealm(pick, false);
            }
        }
        else if (id == "quit") {
            save(p);
            cout << "\n  " << C::MAG << "Your progress is sealed in the spellbook. Farewell, " << p.name << "!" << C::R << "\n";
            return;
        }
    }
}

int Game::run(const string& dataDir) {
    ui::setupConsole();
    vector<string> warnings;
    book = loadRealms(dataDir, warnings);
    if (book.empty()) {
        cerr << "Could not find any realms. Expected a 'content' folder next to the game (looked in: " << dataDir << ").\n"
             << "Set WIZARDS_HOME to the folder that contains content/.\n";
        return 1;
    }
    clearScreen(); ui::banner(); ui::wizardArt();
    for (auto& w : warnings) cout << "  " << C::YEL << "[pack warning] " << w << C::R << "\n";

    if (load(p)) {
        cout << "\n  " << C::GRN << "Welcome back, " << p.name << "! Your spellbook remembers you." << C::R << "\n";
        pause();
    } else {
        cout << "\n  " << C::B << "An old wizard peers at you through crooked spectacles." << C::R << "\n";
        ui::say("  \"Ah, a new apprentice. The ancient language of C++ awaits.\n   Master it, and you shall become an Archmage.\"\n\n");
        cout << "  What is your wizard name? " << C::YEL << "> " << C::R;
        string nm = trim(readLine());
        if (!nm.empty()) p.name = nm.substr(0, 24);
        cout << "\n  \"Welcome, " << C::MAG << p.name << C::R << ". Take this Hint Scroll and Mana Potion. You'll need them.\"\n";
        save(p); pause();
    }
    ui::setTypewriter(p.typewriter); ui::setSound(p.sound);
    mainMenu();
    return 0;
}
