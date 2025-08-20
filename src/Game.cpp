#include "Game.h"
#include <iostream>
#include "raylib.h"
#include "steam/steam_api.h"

Game::Game(int width, int height, const char* title)
    : screenWidth_(width), screenHeight_(height), title_(title),
      currentSceneType_(SceneType::MAIN_MENU) {}

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

    lobbyMgr_ = std::make_unique<LobbyManager>(steamName_);

    InitWindow(screenWidth_, screenHeight_, title_.c_str());

    SetExitKey(KEY_NULL);

    ChangeScene(SceneType::MAIN_MENU);

    return true;
}

void Game::Run() {
    bool shouldQuit = false;
    SceneType lastSceneType = SceneType::MAIN_MENU;

    while (!WindowShouldClose() && !shouldQuit) {
        // Only set FPS when scene type changes
        if (currentSceneType_ != lastSceneType) {
            if (currentSceneType_ == SceneType::GAME) {
                SetTargetFPS(0); // Unlimited FPS for gameplay
            } else {
                SetTargetFPS(60); // Limited FPS for menus
            }
            lastSceneType = currentSceneType_;
        }

        // Get delta time for frame-rate independent updates
        float deltaTime = GetFrameTime();

        if (steamInitialized_) {
            SteamAPI_RunCallbacks();
        }

        currentScene_->Update(deltaTime);

        if (currentSceneType_ == SceneType::MAIN_MENU) {
            MainMenuScene* mainMenu = static_cast<MainMenuScene*>(currentScene_.get());
            if (mainMenu->ShouldStartGame()) {
                ChangeScene(SceneType::GAME);
            }

            if (mainMenu->ShouldQuit()) {
                shouldQuit = true;
            }
        }

        if (currentSceneType_ == SceneType::GAME && IsKeyPressed(KEY_ESCAPE)) {
            if (lobbyMgr_) lobbyMgr_->LeaveLobby();
            ChangeScene(SceneType::MAIN_MENU);
        }

        BeginDrawing();
        currentScene_->Draw();
        EndDrawing();
    }
}

void Game::Shutdown() {
    if (currentScene_) {
        currentScene_->Unload();
        currentScene_.reset();
    }

    if (lobbyMgr_ && lobbyMgr_->IsInLobby()) {
        lobbyMgr_->LeaveLobby();
    }

    if (IsWindowReady()) {
        CloseWindow();
    }
    if (steamInitialized_) {
        SteamAPI_Shutdown();
        steamInitialized_ = false;
    }
}

void Game::ChangeScene(SceneType sceneType) {
    // Unload scene
    if (currentScene_) {
        currentScene_->Unload();
    }

    // Create new scene
    switch (sceneType) {
        case SceneType::MAIN_MENU:
            currentScene_ = std::make_unique<MainMenuScene>(screenWidth_, screenHeight_, steamName_, lobbyMgr_.get());
            break;
        case SceneType::GAME:
            currentScene_ = std::make_unique<GameScene>(screenWidth_, screenHeight_, steamName_, lobbyMgr_.get());
            break;
    }

    currentSceneType_ = sceneType;
    currentScene_->Initialize();
}
