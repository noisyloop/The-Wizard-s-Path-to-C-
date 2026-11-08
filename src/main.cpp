// WIZARDS++  -  The Wizard's Path to C++
#include "engine/game.hpp"
#include "content/content.hpp"

int main(int, char** argv) {
    Game g;
    return g.run(findDataDir(argv[0]));
}
