// Realm content: loaded from content/*.json (core) and packs/*.json (community).
#pragma once
#include <string>
#include <vector>

enum class QType { Choice, Type };

struct Question {
    QType type = QType::Choice;
    std::string prompt, code;
    std::vector<std::string> options;   // Choice
    std::vector<std::string> answers;   // Choice: "b"; Type: accepted answers
    std::string hint, explain;
};

struct SpellFile { std::string name, prelude, epilogue; };

// A real coding challenge. The player types the missing middle of each file;
// the game wraps it with prelude/epilogue, compiles and compares stdout.
struct Spell {
    std::string prompt, expected, hint, explain;
    std::vector<SpellFile> files;   // >=1; files[0] is the program unless mainSrc is set
    std::string mainSrc;            // optional fixed main.cpp (multi-file spells)
    bool valid() const { return !files.empty() && !expected.empty(); }
};

struct Realm {
    std::string id, name, realm, lesson, lessonCode;
    std::vector<Question> trial, pool, boss;
    std::string bossName, bossArt, prize;
    Spell spell;
    bool core = true;          // false = community pack
    std::string source;        // file it came from
};

// Where content/ and packs/ live (searches near the exe, cwd, parents, $WIZARDS_HOME).
std::string findDataDir(const char* argv0);
std::vector<Realm> loadRealms(const std::string& dataDir, std::vector<std::string>& warnings);
