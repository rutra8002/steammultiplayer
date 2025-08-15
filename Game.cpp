#include "Game.h"
#include <iostream>
#include "raylib.h"
#include "steam/steam_api.h"

Game::Game(int width, int height, const char* title)
    : screenWidth_(width), screenHeight_(height), title_(title) {}

Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    steamInitialized_ = SteamAPI_Init();
    if (steamInitialized_) {
        std::cout << "Steam API initialized successfully!" << std::endl;
        if (SteamFriends()) {
            steamName_ = SteamFriends()->GetPersonaName();
        }
    } else {
        std::cerr << "Failed to initialize Steam API!" << std::endl;
        return false;
    }

    InitWindow(screenWidth_, screenHeight_, title_.c_str());

    float px = screenWidth_ / 2.0f;
    float py = screenHeight_ / 2.0f;
    player_ = Player(px, py, 50, 50, steamName_);

    return true;
}

void Game::Run() {
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (steamInitialized_) {
            SteamAPI_RunCallbacks();
        }

        player_.Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        player_.Draw();
        EndDrawing();
    }
}

void Game::Shutdown() {
    if (IsWindowReady()) {
        CloseWindow();
    }
    if (steamInitialized_) {
        SteamAPI_Shutdown();
        steamInitialized_ = false;
    }
}

