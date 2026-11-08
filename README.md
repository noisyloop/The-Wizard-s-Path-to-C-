# 🧙 Wizards++
### *The Wizard's Path to C++*

A gamified terminal adventure that teaches C++ from your very first `cout` to pointers. You're an apprentice climbing through magical realms. Each realm teaches one concept, tests you with trials, then throws a boss at you. Earn XP, gold, ranks and wizard prizes as the spells get harder.

```
  Merlin  |  Adept Conjurer  |  420 XP  |  95 gold  |  Mana ***  |  Streak x3
  Next rank [##########....................] 330 XP to Sorcerer
```

## Play it

Needs a C++17 compiler (for building, and for the **Spell Casting** challenges at runtime). Keep the `content/` folder next to the executable.

**Any platform (g++/clang):**
```
g++ -std=c++17 -O2 -Isrc src/main.cpp src/*/*.cpp -o Wizards && ./Wizards
```
**Windows (MinGW-w64):**
```
g++ -std=c++17 -O2 -static -Isrc src/main.cpp src/*/*.cpp -o Wizards.exe
```
**CMake:** `cmake -B build && cmake --build build` (copies `content/` and `packs/` beside the binary).

Progress saves to `wizards_save.txt`. Best in Windows Terminal.

## How it plays

| Mechanic | What it does |
|---|---|
| **Lessons** | Each realm opens with a short lesson and an example code scroll |
| **Trials** | 3 questions — multiple choice, type-the-answer, and "what does this print?" |
| **Bosses** | Harder questions, double XP, then a real coding spell. Win a wizard prize and unlock the next realm |
| **Spell Casting** | Write real C++; the game compiles it and checks the output. Compiler errors become wizard hints |
| **Practice / Daily / Achievements** | Risk-free replays, one new spell a day with a streak, and unlockable achievements |
| **Mana** | 3 per fight. Wrong answer = −1. Hit 0 and you retreat to try again |
| **Streaks** | 3+ correct in a row = bonus XP |
| **Ranks** | Hedge Apprentice → Rune Scribe → Adept Conjurer → Sorcerer → Warlock of Loops → High Enchanter → **Archmage of C++** |
| **Midnight Bazaar** | Spend gold on Hint Scrolls, Mana Potions, and familiars like the owl *Semicolon* and the phoenix *Segfault* |

## The Realms (15)

| # | Realm | Concept | Boss | Prize |
|---|---|---|---|---|
| 1 | The Novice's Tower | `main`, `cout`, `#include`, `;` | Gribble the Typo Goblin | Wooden Apprentice Wand |
| 2 | The Rune Vault | Variables & types, `const` | The Shapeshifting Mimic | Novice Robe of Runes |
| 3 | The Alchemy Lab | Operators, integer division, `%`, `++` | Sir Cumference the Square Golem | Philosopher's Flask |
| 4 | The Forking Path | `if/else`, `&&` `\|\|` `!`, `switch` | The Two-Headed Riddle Wyrm | Twin-Path Compass |
| 5 | The Looping Labyrinth | `for`, `while`, `do-while`, `break/continue` | Minotaur of Infinite Corridors | Hourglass of Eternal Loops |
| 6 | The Spell Forge | Functions, references, overloading, recursion | Forgemaster Ignis | Ember-Forged Spellbook |
| 7 | The Crystal Archives | Arrays, `std::vector`, range-for | The Crystal Hydra | Prismatic Crystal Orb |
| 8 | The Shadow Realm | Pointers, `&`, `*`, `nullptr` | **The Pointer Lich** | Staff of the Archmage |

---

## 🗺️ Roadmap

### ✅ Phase 0 — Prototype (v0.1, done)
Single-file, playable start to finish. 8 realms, 48 questions, XP/ranks/gold, shop, bosses, prizes, save file, Windows color support.

### Phase 1 — Content split & polish (v0.2)
- Move questions out of code into `content/*.json` so new realms don't need recompiling
- Split code into `ui/`, `engine/`, `content/`, `save/`
- Randomize question order and pull from a bigger pool so replays differ
- "Practice Mode" — replay cleared realms for gold without risk
- Typewriter text effect + sound beeps on correct/wrong
- CMake build

### Phase 2 — Write real spells (v0.3) ⭐ *the big one*
- **Spell Casting challenges**: the player writes actual C++ in a mini editor; the game compiles it (bundled MinGW / TinyCC) and checks the output against expected results
- Boss battles become coding puzzles: *"Write a function that returns the largest rune in a vector."*
- Friendly compiler-error translator: turns cryptic errors into wizard hints

### Phase 3 — Deeper realms (v0.4)
| Realm | Concept |
|---|---|
| The Golem Workshop | Structs & classes, constructors |
| The Bloodline Keep | Inheritance, virtual functions, polymorphism |
| The Summoner's Circle | `new`/`delete`, smart pointers (`unique_ptr`, `shared_ptr`) |
| The Library of Templates | Templates & the STL (`map`, `set`, algorithms) |
| The Chaos Rift | Exceptions & error handling |
| The Time Weavers | Lambdas, iterators, move semantics |
| The Archmage's Spire | Final multi-file project |

### Phase 4 — 2D world (v1.0)
- Move from pure terminal to a 2D tile map with **raylib** (still C++, still one `.exe`)
- Walk your wizard between realm doors; bosses get sprite animations and health bars
- Inventory screen shows earned prizes on your character (robe, hat, staff, familiar)
- Terminal mode kept as an option

### Phase 5 — Community & replay (v1.x)
- Daily Spell: one new challenge a day with a streak counter
- Achievements ("Fixed 10 missing semicolons", "Beat a boss without losing mana")
- Custom realm packs — teachers and players write their own JSON content
- Other languages using the same engine: *Spellbound.py*, *Spellbound.rs*

## Contributing
New questions are the easiest win. Each one needs: a prompt, optional code, the answer(s), a hint, and a one-line "lore" explanation.

## License
MIT
