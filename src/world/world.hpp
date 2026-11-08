// Optional 2D world (raylib). Built only with -DWIZARDS_RAYLIB; otherwise the game is terminal-only.
#pragma once
#include <string>
#include <vector>

#ifdef WIZARDS_RAYLIB
bool worldAvailable();
// Opens the map window. Returns the chosen realm index, or -1 to go back to the terminal menu.
int worldPick(const std::vector<std::string>& realmNames, const std::vector<std::string>& bossNames,
              const std::vector<bool>& open, const std::vector<bool>& cleared,
              const std::vector<std::string>& treasures, const std::string& wizardName);
#else
inline bool worldAvailable() { return false; }
inline int worldPick(const std::vector<std::string>&, const std::vector<std::string>&, const std::vector<bool>&,
                     const std::vector<bool>&, const std::vector<std::string>&, const std::string&) { return -1; }
#endif
