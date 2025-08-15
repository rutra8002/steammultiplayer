#pragma once
#include <string>
#include "Player.h"

class Game {
public:
    Game(int width = 800, int height = 450, const char* title = "yay");
    ~Game();

    bool Init();
    void Run();
    void Shutdown();

private:
    int screenWidth_;
    int screenHeight_;
    std::string title_;

    bool steamInitialized_ = false;
    std::string steamName_ = "Player";

    Player player_;
};

