// Terminal helpers: colors, input, typewriter text, beeps, banner.
#pragma once
#include <string>

namespace C {
extern const char *R, *B, *RED, *GRN, *YEL, *BLU, *MAG, *CYN, *GRY;
}

namespace ui {
void setupConsole();
void clearScreen();
bool interactive();                 // true when stdin is a real terminal
std::string readLine();             // exits cleanly on EOF
void pause();
std::string trim(const std::string& s);
std::string norm(const std::string& s);     // trim + collapse spaces + lowercase
std::string squash(const std::string& s);   // strip all whitespace + lowercase

void setTypewriter(bool on);
void setSound(bool on);
void say(const std::string& text);          // typewriter text (instant if disabled/non-tty)
void beep(bool good);
void banner();
void wizardArt();
void showScroll(const std::string& code);
}
