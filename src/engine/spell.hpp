// Spell casting: compile the player's C++ with the system compiler and check its output.
#pragma once
#include "../content/content.hpp"
#include <string>
#include <vector>

struct SpellResult {
    bool compiled = false, passed = false;
    bool semicolonFix = false;            // compile failed on a missing ';'
    std::string output;                   // program stdout/stderr (trimmed)
    std::string rawErrors;                // first few compiler errors (paths scrubbed)
    std::vector<std::string> wizardHints; // translated, friendly
    std::string note;                     // e.g. timeout
};

bool compilerAvailable();
// userParts[i] is the text the player typed for spell.files[i].
SpellResult castSpell(const Spell& spell, const std::vector<std::string>& userParts);
// Turns gcc/clang diagnostics into wizard hints.
std::vector<std::string> translateErrors(const std::string& raw, bool* semicolon = nullptr);
