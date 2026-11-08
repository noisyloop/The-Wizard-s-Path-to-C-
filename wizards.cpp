// =====================================================================
//  SPELLBOUND++  -  The Wizard's Path to C++
//  A gamified terminal adventure that teaches C++ from zero to pointers.
//
//  Build on Windows (MinGW):   g++ -std=c++17 -O2 -static spellbound.cpp -o Spellbound.exe
//  Build on Windows (MSVC):    cl /std:c++17 /EHsc /O2 spellbound.cpp /Fe:Spellbound.exe
//  Build on Linux/macOS:       g++ -std=c++17 -O2 spellbound.cpp -o spellbound
// =====================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

using namespace std;

// ---------------------------------------------------------------------
//  Colors (ANSI; enabled on Windows 10+ via virtual terminal mode)
// ---------------------------------------------------------------------
namespace C {
const char* R   = "\033[0m";
const char* B   = "\033[1m";
const char* RED = "\033[91m";
const char* GRN = "\033[92m";
const char* YEL = "\033[93m";
const char* BLU = "\033[94m";
const char* MAG = "\033[95m";
const char* CYN = "\033[96m";
const char* GRY = "\033[90m";
}

void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004 /*ENABLE_VIRTUAL_TERMINAL_PROCESSING*/);
    SetConsoleTitleA("Spellbound++  -  The Wizard's Path to C++");
#endif
}

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

// Normalize an answer: trim, collapse whitespace, lowercase.
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

// Same but also strips all spaces (so "int x=5;" == "int x = 5;")
string squash(const string& s) {
    string out;
    for (char ch : s) if (!isspace((unsigned char)ch)) out += (char)tolower((unsigned char)ch);
    return out;
}

// ---------------------------------------------------------------------
//  Content model
// ---------------------------------------------------------------------
enum class QType { Choice, Type };

struct Question {
    QType type;
    string prompt;
    string code;              // optional code snippet shown in a scroll
    vector<string> options;   // for Choice
    vector<string> answers;   // Choice: single letter "b"; Type: accepted answers
    string hint;
    string explain;
};

struct Chapter {
    string name;
    string realm;             // flavour line
    string lesson;            // teaching text shown before the trial
    string lessonCode;        // example code
    vector<Question> trial;
    string bossName;
    string bossArt;
    vector<Question> boss;
    string prize;             // wizard item unlocked
};

Question choice(string p, string code, vector<string> opts, string ans, string hint, string ex) {
    return {QType::Choice, p, code, opts, {ans}, hint, ex};
}
Question typed(string p, string code, vector<string> ans, string hint, string ex) {
    return {QType::Type, p, code, {}, ans, hint, ex};
}

vector<Chapter> buildCurriculum() {
    vector<Chapter> ch;

    // ----------------------------------------------------------- 1
    ch.push_back({
        "The Novice's Tower", "Where every wizard speaks their first words.",
        "Every C++ spell lives inside a function called main(). The program starts there.\n"
        "  To make the computer SPEAK, we use std::cout with the << operator.\n"
        "  #include <iostream> summons the input/output library.\n"
        "  Every statement ends with a semicolon ;  -- forget it and the spell fizzles.",
        "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, Wizard!\" << std::endl;\n    return 0;\n}",
        {
            choice("Where does every C++ program begin running?", "",
                   {"a) start()", "b) main()", "c) begin()", "d) the first line of the file"},
                   "b", "It's the 'main' event.", "Execution always begins in main()."),
            choice("Which line summons the library that provides std::cout?", "",
                   {"a) #include <iostream>", "b) #include <math>", "c) import cout;", "d) using cout;"},
                   "a", "Input + output stream.", "<iostream> = input/output stream library."),
            typed("What character ends almost every C++ statement?", "", {";", "semicolon"},
                  "It looks like a winking comma.", "The semicolon ; ends a statement."),
        },
        "Gribble the Typo Goblin",
        "      ,      ,\n     /(.-\"\"-.)\\\n |\\  \\/      \\/  /|\n | \\ / =.  .= \\ / |\n \\( \\   o\\/o   / )/\n  \\_, '-/  \\-' ,_/\n    /   \\__/   \\\n    \\ \\__/\\__/ /\n  ___\\ \\|--|/ /___\n /`    \\      /    `\\",
        {
            typed("Gribble scrambled your spell! Type the ONE word missing:\n    std::____ << \"Fireball!\";", "",
                  {"cout"}, "Character OUTput.", "std::cout prints to the console."),
            choice("What does this print?", "std::cout << \"A\" << \"B\" << std::endl;",
                   {"a) A B", "b) AB", "c) A\\nB", "d) Error"}, "b",
                   "<< glues things together with no spaces.", "Nothing adds a space, so it prints AB."),
            choice("Gribble deleted something. Why won't this compile?", "int main() {\n    std::cout << \"Hi\"\n}",
                   {"a) Missing return", "b) Missing semicolon", "c) Wrong quotes", "d) main must be void"}, "b",
                   "Look at the end of the cout line.", "The statement needs a ; after \"Hi\". (return 0 is optional in main.)"),
        },
        "Wooden Apprentice Wand"
    });

    // ----------------------------------------------------------- 2
    ch.push_back({
        "The Rune Vault", "Runes store power. Variables store values.",
        "A VARIABLE is a named box that holds a value. You must declare its TYPE:\n"
        "    int    -> whole numbers      (42)\n"
        "    double -> decimal numbers    (3.14)\n"
        "    char   -> one character      ('A')  single quotes!\n"
        "    bool   -> true or false\n"
        "    std::string -> text          (\"Merlin\")  needs #include <string>\n"
        "  'const' makes a rune unchangeable after it's set.",
        "int mana = 100;\ndouble power = 7.5;\nchar house = 'R';\nbool isWizard = true;\nstd::string name = \"Merlin\";\nconst int MAX_LEVEL = 99;",
        {
            choice("Which type should store the number of spells known (e.g. 12)?", "",
                   {"a) double", "b) bool", "c) int", "d) char"}, "c",
                   "A whole number.", "int holds whole numbers."),
            choice("Which line is a VALID char?", "",
                   {"a) char c = \"Z\";", "b) char c = 'Z';", "c) char c = Z;", "d) char c = 'ZZ';"}, "b",
                   "Single quotes, single character.", "chars use single quotes and hold exactly one character."),
            typed("Declare an int named gold with value 50. (Type the full line.)", "",
                  {"int gold = 50;", "int gold=50;", "int gold{50};", "int gold(50);"},
                  "type name = value;", "int gold = 50;  (int gold{50}; also works - brace init!)"),
        },
        "The Shapeshifting Mimic",
        "    ________________\n   /                \\\n  |  []        []    |\n  |    \\/\\/\\/\\/\\/    |\n  |   /\\/\\/\\/\\/\\/    |\n   \\________________/\n    |   TREASURE   |\n    |______________|",
        {
            choice("The Mimic tries to change a const. What happens?", "const int lives = 3;\nlives = 4;",
                   {"a) lives becomes 4", "b) Compile error", "c) lives becomes 7", "d) Runtime crash"}, "b",
                   "const means constant.", "Assigning to a const is a compile-time error."),
            choice("What's stored in x?", "int x = 7.9;",
                   {"a) 7.9", "b) 8", "c) 7", "d) Error"}, "c",
                   "ints don't round, they chop.", "Converting double to int truncates toward zero: 7."),
            typed("What type holds true/false? (one word)", "", {"bool"},
                  "Named after George Boole.", "bool holds true or false."),
        },
        "Novice Robe of Runes"
    });

    // ----------------------------------------------------------- 3
    ch.push_back({
        "The Alchemy Lab", "Mix numbers, transmute values.",
        "Operators combine values:  +  -  *  /  and  %  (remainder / modulo).\n"
        "  WARNING: int / int is INTEGER division. 7 / 2 == 3, not 3.5!\n"
        "  7 % 2 == 1 (the leftover).  Use 7.0 / 2 to get 3.5.\n"
        "  x += 5 means x = x + 5.   x++ adds 1 to x.\n"
        "  Comparisons give bool:  ==  !=  <  >  <=  >=",
        "int potions = 10;\npotions += 5;      // 15\npotions++;         // 16\nint left = 17 % 5; // 2\ndouble avg = 7.0 / 2; // 3.5",
        {
            choice("What is 17 / 5 in C++ (both ints)?", "", {"a) 3.4", "b) 3", "c) 4", "d) 2"}, "b",
                   "Integer division drops the decimal.", "int / int truncates: 17/5 == 3."),
            typed("What is 17 % 5 ?", "", {"2"}, "What's left over after 5 goes into 17?", "17 = 5*3 + 2, so 17 % 5 == 2."),
            choice("Which operator checks EQUALITY?", "", {"a) =", "b) ==", "c) ===", "d) :="}, "b",
                   "One = assigns. Two compares.", "== compares; = assigns. (=== is JavaScript!)"),
        },
        "Sir Cumference the Square Golem",
        "     ___________\n    |  _     _  |\n    | |O|   |O| |\n    |     ^     |\n    |  \\_____/  |\n    |___________|\n   /|           |\\\n  /_|   10 x 10 |_\\\n    |___________|\n     |_|     |_|",
        {
            typed("What does this print?", "int a = 5;\na *= 2;\na -= 3;\nstd::cout << a;", {"7"},
                  "5 -> times 2 -> minus 3.", "5*2 = 10, 10-3 = 7."),
            choice("Which gives 2.5?", "", {"a) 5 / 2", "b) 5 % 2", "c) 5.0 / 2", "d) (int)5.0 / 2"}, "c",
                   "At least one side must be a double.", "5.0 / 2 is floating-point division = 2.5."),
            typed("What does this print?", "int x = 3;\nint y = x++ + 10;\nstd::cout << y << \" \" << x;", {"13 4"},
                  "Post-increment: use first, then add.", "x++ yields 3 THEN makes x 4, so y = 13 and x = 4."),
        },
        "Bubbling Philosopher's Flask"
    });

    // ----------------------------------------------------------- 4
    ch.push_back({
        "The Forking Path", "Every choice opens a new road.",
        "if / else lets your spell DECIDE:\n"
        "    if (mana >= 10) { cast(); } else { rest(); }\n"
        "  Combine conditions:  &&  (AND)   ||  (OR)   !  (NOT)\n"
        "  switch picks among many exact values - don't forget 'break;'!",
        "if (mana >= 50 && level > 3) {\n    std::cout << \"Meteor Storm!\";\n} else if (mana >= 10) {\n    std::cout << \"Spark.\";\n} else {\n    std::cout << \"Too tired...\";\n}",
        {
            choice("What prints when mana = 20, level = 5?", "if (mana >= 50 && level > 3) cout << \"Meteor\";\nelse if (mana >= 10) cout << \"Spark\";\nelse cout << \"Tired\";",
                   {"a) Meteor", "b) Spark", "c) Tired", "d) MeteorSpark"}, "b",
                   "&& needs BOTH to be true.", "mana >= 50 is false, so the first branch fails; mana >= 10 is true -> Spark."),
            typed("Which operator means OR? (type it)", "", {"||", "or"}, "Two vertical bars.", "|| is logical OR (the keyword 'or' also works!)."),
            choice("What is !true ?", "", {"a) true", "b) false", "c) 0.5", "d) Error"}, "b",
                   "! flips it.", "! negates: !true == false."),
        },
        "The Two-Headed Riddle Wyrm",
        "        __    __\n       /oo\\  /oo\\\n       \\  /  \\  /\n        )(    )(\n       (  \\__/  )\n        \\      /\n    ~~~~~\\    /~~~~~\n          \\__/",
        {
            choice("The Wyrm forgot a break. What prints for spell = 1?", "switch (spell) {\n  case 1: cout << \"Fire \";\n  case 2: cout << \"Ice \";\n          break;\n  case 3: cout << \"Bolt \";\n}",
                   {"a) Fire", "b) Fire Ice", "c) Fire Ice Bolt", "d) Ice"}, "b",
                   "Without break, execution FALLS THROUGH.", "case 1 falls through into case 2, then hits break: 'Fire Ice'."),
            choice("Left head: What prints?", "int hp = 0;\nif (hp = 5) cout << \"Alive\";\nelse cout << \"Dead\";",
                   {"a) Dead", "b) Alive", "c) Compile error", "d) Nothing"}, "b",
                   "Look VERY carefully at the = sign.", "hp = 5 ASSIGNS 5 (non-zero = true), so Alive prints. Classic bug!"),
            typed("Right head: What does this evaluate to? (true/false)", "(3 > 2) && !(5 == 5) || (1 < 2)", {"true", "1"},
                  "&& binds tighter than ||.", "(true && false) || true  ->  false || true  ->  true."),
        },
        "Twin-Path Compass"
    });

    // ----------------------------------------------------------- 5
    ch.push_back({
        "The Looping Labyrinth", "Repeat the incantation until the door opens.",
        "Loops repeat code:\n"
        "    for (int i = 0; i < 5; i++) { ... }   // runs 5 times: i = 0..4\n"
        "    while (condition) { ... }              // runs while true\n"
        "    do { ... } while (condition);          // runs at least once\n"
        "  'break' escapes the loop.  'continue' skips to the next round.",
        "for (int i = 1; i <= 3; i++) {\n    std::cout << \"Chant \" << i << \"\\n\";\n}\n// Chant 1\n// Chant 2\n// Chant 3",
        {
            typed("How many times does this loop run?", "for (int i = 0; i < 5; i++) { }", {"5", "five"},
                  "Count: 0, 1, 2, ...", "i goes 0,1,2,3,4 -> 5 times."),
            typed("What does this print?", "for (int i = 0; i < 3; i++) cout << i;", {"012"},
                  "No spaces are printed.", "Prints 0, then 1, then 2 -> 012."),
            choice("Which loop ALWAYS runs at least once?", "", {"a) for", "b) while", "c) do-while", "d) none"}, "c",
                   "It checks the condition at the END.", "do-while checks after the body, so it runs at least once."),
        },
        "The Minotaur of Infinite Corridors",
        "        (    )\n       (__)(__)\n      /  o  o  \\\n     |    __    |\n      \\  (__)  /\n    ___\\______/___\n   /    |    |    \\\n  /  /| |    | |\\  \\\n     \\|_|____|_|/",
        {
            typed("What does this print?", "for (int i = 0; i < 6; i++) {\n    if (i == 4) break;\n    if (i % 2 == 0) continue;\n    cout << i;\n}", {"13"},
                  "continue skips evens. break stops at 4.", "0 skipped, 1 printed, 2 skipped, 3 printed, 4 -> break. Output: 13"),
            choice("Why is this loop dangerous?", "int i = 0;\nwhile (i < 10) {\n    cout << i;\n}",
                   {"a) It prints nothing", "b) It never ends", "c) Compile error", "d) It runs 10 times"}, "b",
                   "Does i ever change?", "i is never incremented: infinite loop! The Minotaur's corridor never ends."),
            typed("What is the final value of sum?", "int sum = 0;\nfor (int i = 1; i <= 4; i++) sum += i;", {"10"},
                  "1 + 2 + 3 + 4", "1+2+3+4 = 10."),
        },
        "Hourglass of Eternal Loops"
    });

    // ----------------------------------------------------------- 6
    ch.push_back({
        "The Spell Forge", "Forge reusable spells: functions.",
        "A FUNCTION is a named, reusable spell:\n"
        "    int heal(int hp, int amount) { return hp + amount; }\n"
        "  return type  name  (parameters)  { body }\n"
        "  'void' means it returns nothing.\n"
        "  Parameters are COPIES by default. Use & (reference) to modify the original:\n"
        "    void boost(int& mana) { mana += 10; }",
        "int square(int n) {\n    return n * n;\n}\n\nvoid greet(std::string name) {\n    std::cout << \"Hail, \" << name;\n}\n\nint main() {\n    int s = square(4);   // 16\n    greet(\"Morgana\");\n}",
        {
            choice("What is the return type of: void chant()", "", {"a) int", "b) nothing", "c) string", "d) chant"}, "b",
                   "void = emptiness.", "void functions return nothing."),
            typed("What does this print?", "int twice(int x) { return x * 2; }\n...\ncout << twice(twice(3));", {"12"},
                  "Work from the inside out.", "twice(3) = 6, twice(6) = 12."),
            choice("Which signature lets the function CHANGE the caller's variable?", "",
                   {"a) void f(int x)", "b) void f(int& x)", "c) void f(const int x)", "d) int f()"}, "b",
                   "& means reference.", "int& x is a reference to the original variable."),
        },
        "Forgemaster Ignis, the Fire Elemental",
        "          )  (\n         (   ) )\n          ) ( (\n        _(_____)_\n       /  ^   ^  \\\n      |  (o) (o)  |\n      |    ___    |\n       \\  \\___/  /\n        \\_______/\n       ~~~~~~~~~~~",
        {
            typed("Ignis tests copies vs references. What prints?", "void a(int x)  { x = 99; }\nvoid b(int& x) { x = 42; }\n...\nint m = 1;\na(m);\ncout << m << \" \";\nb(m);\ncout << m;", {"1 42"},
                  "Only one of these touches the real m.", "a() changes a copy (m stays 1); b() changes m via reference -> 42. Output: 1 42"),
            choice("What is this called?", "int power(int x)           { return x; }\nint power(int x, int y)    { return x * y; }",
                   {"a) Recursion", "b) Overloading", "c) Inheritance", "d) Error: duplicate name"}, "b",
                   "Same name, different parameters.", "Function overloading: same name, different parameter lists."),
            typed("Recursion! What does fact(4) return?", "int fact(int n) {\n    if (n <= 1) return 1;\n    return n * fact(n - 1);\n}", {"24"},
                  "4 * 3 * 2 * 1", "4! = 24. A function calling itself is recursion."),
        },
        "Ember-Forged Spellbook"
    });

    // ----------------------------------------------------------- 7
    ch.push_back({
        "The Crystal Archives", "Store many values in a row of crystals.",
        "An ARRAY holds many values of one type:  int runes[3] = {5, 8, 2};\n"
        "  Indexes start at ZERO:  runes[0] == 5,  runes[2] == 2.\n"
        "  std::vector is a magical array that can GROW:  (#include <vector>)\n"
        "    std::vector<int> v = {1, 2};   v.push_back(3);   v.size() == 3\n"
        "  Range-for loops visit each element:  for (int r : v) { ... }",
        "std::vector<std::string> spells = {\"Fire\", \"Ice\"};\nspells.push_back(\"Lightning\");\nfor (const auto& s : spells) {\n    std::cout << s << \"\\n\";\n}\nstd::cout << spells.size(); // 3",
        {
            typed("What's the index of the FIRST element of an array?", "", {"0", "zero"},
                  "Programmers start counting early.", "Arrays are zero-indexed."),
            typed("What does this print?", "int a[4] = {10, 20, 30, 40};\ncout << a[2];", {"30"},
                  "Index 0 is 10...", "a[0]=10, a[1]=20, a[2]=30."),
            choice("How do you add an element to the end of a vector v?", "",
                   {"a) v.add(5)", "b) v.push_back(5)", "c) v[v.size()] = 5", "d) v += 5"}, "b",
                   "Push it to the back.", "push_back appends to the end."),
        },
        "The Crystal Hydra",
        "     /\\    /\\    /\\\n    /oo\\  /oo\\  /oo\\\n    \\  /  \\  /  \\  /\n     ||    ||    ||\n      \\\\   ||   //\n       \\\\  ||  //\n        \\\\_||_//\n        /      \\\n       /________\\",
        {
            choice("The Hydra's array has 5 heads. What's wrong?", "int heads[5];\nheads[5] = 1;",
                   {"a) Nothing", "b) Out of bounds - valid indexes are 0..4", "c) Must use vector", "d) Arrays can't hold 1"}, "b",
                   "5 elements: indexes 0 to ?", "heads[5] is one past the end: undefined behaviour!"),
            typed("What does this print?", "vector<int> v = {3, 1, 4};\nv.push_back(1);\nv.push_back(5);\ncout << v.size() << v[3];", {"51"},
                  "Count them, then find index 3.", "v = {3,1,4,1,5}: size 5, v[3] = 1 -> prints 51."),
            typed("What is total?", "vector<int> v = {2, 4, 6};\nint total = 0;\nfor (int x : v) total += x * x;", {"56"},
                  "Square each, then add.", "4 + 16 + 36 = 56."),
        },
        "Prismatic Crystal Orb"
    });

    // ----------------------------------------------------------- 8
    ch.push_back({
        "The Shadow Realm", "Here, wizards point at memory itself.",
        "Every variable lives at a MEMORY ADDRESS.\n"
        "    &x   -> the address of x\n"
        "    int* p = &x;   -> p is a POINTER that stores x's address\n"
        "    *p   -> DEREFERENCE: the value at that address (it IS x)\n"
        "  nullptr means 'points to nothing'. Never dereference it!\n"
        "  Changing *p changes x - they are the same box.",
        "int mana = 50;\nint* p = &mana;\n*p = 80;\nstd::cout << mana;   // 80 - changed through the pointer!",
        {
            choice("What does &x give you?", "", {"a) x's value", "b) x's address", "c) A reference", "d) x doubled"}, "b",
                   "& in an expression = address-of.", "&x is the memory address of x."),
            typed("What does this print?", "int x = 7;\nint* p = &x;\ncout << *p;", {"7"},
                  "*p looks inside the address.", "*p dereferences to x, which is 7."),
            typed("What keyword means 'a pointer to nothing'?", "", {"nullptr"},
                  "null + ptr", "nullptr is the modern null pointer."),
        },
        "The Pointer Lich (FINAL BOSS)",
        "            .-\"\"\"-.\n           /  _ _  \\\n           | (o)(o) |\n           |   ^    |\n           |  \\___/ |\n            \\______/\n         .-'  |  |  '-.\n        / *p  |  | &x  \\\n       |  ~~~~~~~~~~~~  |\n        \\_/ SHADOW   \\_/",
        {
            typed("The Lich twists memory. What prints?", "int a = 10, b = 20;\nint* p = &a;\n*p = 15;\np = &b;\n*p += 5;\ncout << a << \" \" << b;", {"15 25"},
                  "Track where p points at each step.", "p->a: a=15. p->b: b+=5 -> 25. Output: 15 25"),
            choice("What happens here?", "int* p = nullptr;\ncout << *p;",
                   {"a) Prints 0", "b) Prints nullptr", "c) Undefined behaviour / crash", "d) Compile error"}, "c",
                   "There's nothing there to look at.", "Dereferencing nullptr is undefined behaviour - usually a crash."),
            typed("Final spell! What prints?", "void swap(int* a, int* b) {\n    int t = *a; *a = *b; *b = t;\n}\n...\nint x = 1, y = 2;\nswap(&x, &y);\ncout << x << y;", {"21"},
                  "The function swaps through pointers.", "Pointers let swap() change the originals: x=2, y=1 -> prints 21."),
        },
        "Staff of the Archmage"
    });

    return ch;
}

// ---------------------------------------------------------------------
//  Player & persistence
// ---------------------------------------------------------------------
struct Player {
    string name = "Apprentice";
    int xp = 0;
    int gold = 0;
    int unlocked = 0;          // highest chapter index unlocked
    int hintScrolls = 1;
    int potions = 1;
    int bestStreak = 0;
    set<int> cleared;          // chapters whose boss is defeated
    vector<string> treasures;  // prizes + shop cosmetics
};

const char* SAVE_FILE = "spellbound_save.txt";

void save(const Player& p) {
    ofstream f(SAVE_FILE);
    f << p.name << "\n" << p.xp << " " << p.gold << " " << p.unlocked << " "
      << p.hintScrolls << " " << p.potions << " " << p.bestStreak << "\n";
    f << p.cleared.size(); for (int c : p.cleared) f << " " << c; f << "\n";
    f << p.treasures.size() << "\n"; for (auto& t : p.treasures) f << t << "\n";
}

bool load(Player& p) {
    ifstream f(SAVE_FILE);
    if (!f) return false;
    getline(f, p.name);
    f >> p.xp >> p.gold >> p.unlocked >> p.hintScrolls >> p.potions >> p.bestStreak;
    size_t n; f >> n; p.cleared.clear();
    for (size_t i = 0; i < n; i++) { int c; f >> c; p.cleared.insert(c); }
    f >> n; f.ignore(10000, '\n'); p.treasures.clear();
    for (size_t i = 0; i < n; i++) { string t; getline(f, t); p.treasures.push_back(t); }
    return bool(f) || f.eof();
}

struct Rank { int xp; const char* title; const char* color; };
const vector<Rank> RANKS = {
    {0,    "Hedge Apprentice", C::GRY},
    {150,  "Rune Scribe",      C::GRN},
    {400,  "Adept Conjurer",   C::CYN},
    {750,  "Sorcerer",         C::BLU},
    {1200, "Warlock of Loops", C::MAG},
    {1800, "High Enchanter",   C::YEL},
    {2600, "ARCHMAGE OF C++",  C::RED},
};

int rankIndex(int xp) { int r = 0; for (size_t i = 0; i < RANKS.size(); i++) if (xp >= RANKS[i].xp) r = (int)i; return r; }

// ---------------------------------------------------------------------
//  UI pieces
// ---------------------------------------------------------------------
void banner() {
    cout << C::MAG << C::B <<
R"(
       _____ ____  ________    __    ____  ____  __  ___   ______   __      __
      / ___// __ \/ ____/ /   / /   / __ )/ __ \/ / / / | / / __ \_/ /_  __/ /_
      \__ \/ /_/ / __/ / /   / /   / __  / / / / / / /  |/ / / / /_  __//_  __/
     ___/ / ____/ /___/ /___/ /___/ /_/ / /_/ / /_/ / /|  / /_/ / /_/    /_/
    /____/_/   /_____/_____/_____/_____/\____/\____/_/ |_/_____/
)" << C::R << C::YEL << "                       ~ The Wizard's Path to C++ ~\n" << C::R;
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

void hud(const Player& p, int mana = -1, int streak = -1) {
    int r = rankIndex(p.xp);
    cout << C::GRY << "  -----------------------------------------------------------------\n" << C::R;
    cout << "  " << C::B << p.name << C::R << "  |  " << RANKS[r].color << RANKS[r].title << C::R
         << "  |  " << C::CYN << p.xp << " XP" << C::R
         << "  |  " << C::YEL << p.gold << " gold" << C::R;
    if (mana >= 0) {
        cout << "  |  " << C::BLU << "Mana ";
        for (int i = 0; i < mana; i++) cout << "*";
        cout << C::R;
    }
    if (streak > 1) cout << "  |  " << C::RED << "Streak x" << streak << C::R;
    cout << "\n";
    if (r + 1 < (int)RANKS.size()) {
        int lo = RANKS[r].xp, hi = RANKS[r + 1].xp;
        int filled = (p.xp - lo) * 30 / (hi - lo);
        cout << "  Next rank [" << C::CYN;
        for (int i = 0; i < 30; i++) cout << (i < filled ? '#' : '.');
        cout << C::R << "] " << (hi - p.xp) << " XP to " << RANKS[r + 1].title << "\n";
    }
    cout << C::GRY << "  -----------------------------------------------------------------\n" << C::R;
}

void showScroll(const string& code) {
    if (code.empty()) return;
    cout << C::YEL << "    .--------------------------------------------.\n" << C::R;
    stringstream ss(code); string line;
    while (getline(ss, line)) cout << C::YEL << "    | " << C::GRN << line << C::R << "\n";
    cout << C::YEL << "    '--------------------------------------------'\n" << C::R;
}

// ---------------------------------------------------------------------
//  Question engine
// ---------------------------------------------------------------------
// Returns: 1 correct, 0 wrong. Handles hints and potions.
int ask(Player& p, const Question& q, int num, int total, int& mana, int streak, bool bossMode) {
    bool hinted = false;
    while (true) {
        cout << "\n  " << (bossMode ? C::RED : C::CYN) << C::B
             << (bossMode ? "BOSS ATTACK " : "TRIAL ") << num << "/" << total << C::R << "\n";
        cout << "  " << C::B << q.prompt << C::R << "\n";
        showScroll(q.code);
        for (auto& o : q.options) cout << "      " << o << "\n";
        if (hinted) cout << "  " << C::MAG << "Hint: " << q.hint << C::R << "\n";
        cout << C::GRY << "  (type 'hint' to use a Hint Scroll [" << p.hintScrolls << "], 'potion' to drink a Mana Potion ["
             << p.potions << "])\n" << C::R;
        cout << "  " << C::YEL << "> " << C::R;
        string in = readLine();
        string n = norm(in);

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
            ok = !n.empty() && n[0] == q.answers[0][0] && (n.size() == 1 || n[1] == ')' || n[1] == ' ');
        } else {
            for (auto& a : q.answers) if (norm(a) == n || squash(a) == squash(in)) ok = true;
        }

        if (ok) {
            const char* cheers[] = {"The runes glow!", "Spell cast flawlessly!", "Sparks fly from your wand!",
                                    "The ancient tome approves.", "Brilliant sorcery!", "Your familiar hoots in delight!"};
            cout << "  " << C::GRN << C::B << "+ " << cheers[(num + streak) % 6] << C::R << "\n";
        } else {
            mana--;
            cout << "  " << C::RED << C::B << "x The spell backfires! -1 Mana" << C::R << "\n";
            if (q.type == QType::Choice) cout << "  Correct answer: " << C::GRN << q.answers[0] << C::R << "\n";
            else cout << "  Correct answer: " << C::GRN << q.answers[0] << C::R << "\n";
        }
        cout << "  " << C::GRY << "Lore: " << q.explain << C::R << "\n";
        return ok ? 1 : 0;
    }
}

void levelUpCheck(int before, int after) {
    int a = rankIndex(before), b = rankIndex(after);
    if (b > a) {
        cout << "\n" << C::YEL << C::B
             << "  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *\n"
             << "     RANK UP!  You are now: " << RANKS[b].title << "\n"
             << "  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *" << C::R << "\n";
    }
}

// Runs a set of questions. Returns true if survived (mana > 0 at end).
bool runSet(Player& p, const vector<Question>& qs, int chapterIdx, bool boss, int startMana, int& correctOut) {
    int mana = startMana, streak = 0;
    correctOut = 0;
    int baseXP = 10 + chapterIdx * 5;
    if (boss) baseXP *= 2;
    for (size_t i = 0; i < qs.size(); i++) {
        hud(p, mana, streak);
        int before = p.xp;
        if (ask(p, qs[i], (int)i + 1, (int)qs.size(), mana, streak, boss)) {
            streak++;
            p.bestStreak = max(p.bestStreak, streak);
            int bonus = streak >= 3 ? baseXP / 2 : 0;
            p.xp += baseXP + bonus;
            p.gold += 5 + chapterIdx * 2;
            correctOut++;
            cout << "  " << C::CYN << "+" << baseXP << " XP" << C::R;
            if (bonus) cout << C::RED << "  +" << bonus << " streak bonus!" << C::R;
            cout << C::YEL << "  +" << (5 + chapterIdx * 2) << " gold" << C::R << "\n";
            levelUpCheck(before, p.xp);
        } else {
            streak = 0;
        }
        if (mana <= 0) {
            cout << "\n  " << C::RED << C::B << "Your mana is spent. You collapse..." << C::R << "\n";
            return false;
        }
        pause();
        clearScreen();
    }
    return true;
}

// ---------------------------------------------------------------------
//  Game screens
// ---------------------------------------------------------------------
void playChapter(Player& p, const vector<Chapter>& book, int idx) {
    const Chapter& c = book[idx];
    clearScreen();
    cout << C::MAG << C::B << "\n  ~~~ Chapter " << idx + 1 << ": " << c.name << " ~~~" << C::R << "\n";
    cout << "  " << C::GRY << c.realm << C::R << "\n\n";
    cout << C::CYN << C::B << "  THE MASTER'S LESSON" << C::R << "\n  " << c.lesson << "\n\n";
    showScroll(c.lessonCode);
    pause();
    clearScreen();

    int correct = 0;
    cout << C::CYN << C::B << "\n  ~ The Trials of " << c.name << " ~" << C::R << "\n";
    bool survived = runSet(p, c.trial, idx, false, 3, correct);
    if (!survived) { cout << "  Study the lesson and return stronger.\n"; save(p); pause(); return; }

    clearScreen();
    cout << C::RED << C::B << "\n  A shadow falls across the chamber...\n\n" << C::R;
    cout << C::RED << c.bossArt << C::R << "\n\n";
    cout << "  " << C::B << c.bossName << C::R << " blocks your path!\n";
    cout << "  " << C::GRY << "(Boss battles give double XP. You have 3 Mana.)" << C::R << "\n";
    pause();
    clearScreen();

    bool won = runSet(p, c.boss, idx, true, 3, correct);
    if (!won) {
        cout << "  " << c.bossName << " laughs as you retreat. Try again!\n";
        save(p); pause(); return;
    }

    clearScreen();
    bool first = !p.cleared.count(idx);
    cout << C::YEL << C::B << "\n  ****************************************************\n"
         << "     VICTORY!  " << c.bossName << " is defeated!\n"
         << "  ****************************************************" << C::R << "\n\n";
    if (first) {
        p.cleared.insert(idx);
        p.treasures.push_back(c.prize);
        int before = p.xp;
        p.xp += 50; p.gold += 40;
        cout << "  " << C::MAG << "You found: " << C::B << c.prize << C::R << "\n";
        cout << "  " << C::CYN << "+50 XP" << C::R << "   " << C::YEL << "+40 gold" << C::R << "\n";
        levelUpCheck(before, p.xp);
        if (idx + 1 < (int)book.size() && p.unlocked < idx + 1) {
            p.unlocked = idx + 1;
            cout << "\n  " << C::GRN << "A new realm opens: " << book[idx + 1].name << C::R << "\n";
        }
        if (idx + 1 == (int)book.size()) {
            cout << "\n" << C::YEL << C::B
                 << "  You have conquered every realm of Spellbound++.\n"
                 << "  The Council of Code bows before you, ARCHMAGE.\n"
                 << "  (Next step: open a real compiler and write your own spells!)" << C::R << "\n";
        }
    } else {
        cout << "  You've beaten this boss before. Practice makes perfect! (XP kept)\n";
    }
    save(p);
    pause();
}

void bazaar(Player& p) {
    struct Item { string name; int cost; int kind; }; // 0 hint, 1 potion, 2 cosmetic
    vector<Item> items = {
        {"Hint Scroll", 15, 0},
        {"Mana Potion", 25, 1},
        {"Owl Familiar 'Semicolon'", 60, 2},
        {"Hat of Infinite Recursion", 90, 2},
        {"Cloak of Compile-Time Safety", 120, 2},
        {"Phoenix Familiar 'Segfault'", 200, 2},
    };
    while (true) {
        clearScreen();
        cout << C::YEL << C::B << "\n  ~ The Midnight Bazaar ~" << C::R << "\n";
        cout << C::GRY << "  A hooded merchant grins. \"Gold for magic, wizard?\"" << C::R << "\n\n";
        hud(p);
        for (size_t i = 0; i < items.size(); i++) {
            bool owned = items[i].kind == 2 && find(p.treasures.begin(), p.treasures.end(), items[i].name) != p.treasures.end();
            cout << "   " << i + 1 << ") " << items[i].name << "  " << C::YEL << items[i].cost << "g" << C::R
                 << (owned ? string(C::GRN) + "  (owned)" + C::R : "") << "\n";
        }
        cout << "   0) Leave\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "0" || s.empty()) return;
        int k = atoi(s.c_str()) - 1;
        if (k < 0 || k >= (int)items.size()) continue;
        Item& it = items[k];
        bool owned = it.kind == 2 && find(p.treasures.begin(), p.treasures.end(), it.name) != p.treasures.end();
        if (owned) { cout << "  You already own that.\n"; pause(); continue; }
        if (p.gold < it.cost) { cout << "  " << C::RED << "Not enough gold." << C::R << "\n"; pause(); continue; }
        p.gold -= it.cost;
        if (it.kind == 0) p.hintScrolls++;
        else if (it.kind == 1) p.potions++;
        else p.treasures.push_back(it.name);
        cout << "  " << C::GRN << "Purchased " << it.name << "!" << C::R << "\n";
        save(p);
        pause();
    }
}

void trophyRoom(const Player& p, const vector<Chapter>& book) {
    clearScreen();
    cout << C::MAG << C::B << "\n  ~ Your Wizard's Chamber ~" << C::R << "\n\n";
    hud(p);
    cout << "  Realms conquered: " << p.cleared.size() << "/" << book.size() << "\n";
    cout << "  Best streak:      " << p.bestStreak << "\n";
    cout << "  Hint Scrolls:     " << p.hintScrolls << "     Mana Potions: " << p.potions << "\n\n";
    cout << C::YEL << "  TREASURES" << C::R << "\n";
    if (p.treasures.empty()) cout << "   (empty - defeat bosses to earn wizard prizes!)\n";
    for (auto& t : p.treasures) cout << "   * " << t << "\n";
    cout << "\n" << C::YEL << "  RANKS" << C::R << "\n";
    for (auto& r : RANKS)
        cout << "   " << (p.xp >= r.xp ? C::GRN : C::GRY) << (p.xp >= r.xp ? "[x] " : "[ ] ")
             << r.title << "  (" << r.xp << " XP)" << C::R << "\n";
    pause();
}

void mapScreen(Player& p, const vector<Chapter>& book) {
    while (true) {
        clearScreen();
        cout << C::CYN << C::B << "\n  ~ The Map of Realms ~" << C::R << "\n\n";
        hud(p);
        for (size_t i = 0; i < book.size(); i++) {
            bool open = (int)i <= p.unlocked;
            bool done = p.cleared.count((int)i);
            cout << "   " << i + 1 << ") ";
            if (done) cout << C::GRN << "[*] ";
            else if (open) cout << C::YEL << "[ ] ";
            else cout << C::GRY << "[locked] ";
            cout << book[i].name << C::R;
            if (open && !done) cout << C::YEL << "   <- your next quest" << C::R;
            cout << "\n";
            if (i + 1 < book.size()) cout << C::GRY << "       |" << C::R << "\n";
        }
        cout << "\n   0) Back\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "0" || s.empty()) return;
        int k = atoi(s.c_str()) - 1;
        if (k < 0 || k >= (int)book.size()) continue;
        if (k > p.unlocked) { cout << "  " << C::RED << "A magic barrier blocks the way. Defeat the previous boss first." << C::R << "\n"; pause(); continue; }
        playChapter(p, book, k);
    }
}

int main() {
    setupConsole();
    vector<Chapter> book = buildCurriculum();
    Player p;

    clearScreen();
    banner();
    wizardArt();

    if (load(p)) {
        cout << "\n  " << C::GRN << "Welcome back, " << p.name << "! Your spellbook remembers you." << C::R << "\n";
        pause();
    } else {
        cout << "\n  " << C::B << "An old wizard peers at you through crooked spectacles." << C::R << "\n";
        cout << "  \"Ah, a new apprentice. The ancient language of C++ awaits.\n";
        cout << "   Master it, and you shall become an Archmage.\"\n\n";
        cout << "  What is your wizard name? " << C::YEL << "> " << C::R;
        string nm = trim(readLine());
        if (!nm.empty()) p.name = nm.substr(0, 24);
        cout << "\n  \"Welcome, " << C::MAG << p.name << C::R << ". Take this Hint Scroll and Mana Potion. You'll need them.\"\n";
        save(p);
        pause();
    }

    while (true) {
        clearScreen();
        banner();
        hud(p);
        cout << "\n   1) " << C::CYN << "Journey" << C::R << "         - continue your quest\n";
        cout << "   2) " << C::YEL << "Midnight Bazaar" << C::R << " - spend gold on hints, potions, familiars\n";
        cout << "   3) " << C::MAG << "Wizard's Chamber" << C::R << " - treasures, ranks and stats\n";
        cout << "   4) " << C::GRY << "Leave the tower" << C::R << "\n";
        cout << "\n  " << C::YEL << "> " << C::R;
        string s = trim(readLine());
        if (s == "1") mapScreen(p, book);
        else if (s == "2") bazaar(p);
        else if (s == "3") trophyRoom(p, book);
        else if (s == "4") {
            save(p);
            cout << "\n  " << C::MAG << "Your progress is sealed in the spellbook. Farewell, " << p.name << "!" << C::R << "\n";
            return 0;
        }
    }
}
