#include <iostream>
#include "src/Game.h"

int main() {
    Game game(800, 450, "yay");
    if (!game.Init()) {
        std::cerr << "Initialization failed." << std::endl;
        return 1;
    }
    game.Run();
    return 0;
}