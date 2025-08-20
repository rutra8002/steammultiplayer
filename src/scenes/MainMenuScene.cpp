#include "MainMenuScene.h"
#include "raylib.h"

MainMenuScene::MainMenuScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby)
    : quitButton(screenWidth / 2.0f - 100, screenHeight / 2.0f + 150, 200, 50, "Quit Game", 20,
                 BLACK, LIGHTGRAY, GRAY, DARKGRAY),
      hostButton(screenWidth / 2.0f - 310, screenHeight / 2.0f + 20, 200, 50, "Host Lobby", 20,
                 BLACK, LIGHTGRAY, GRAY, DARKGRAY),
      refreshButton(screenWidth / 2.0f - 100, screenHeight / 2.0f + 20, 200, 50, "Refresh Lobbies", 20,
                 BLACK, LIGHTGRAY, GRAY, DARKGRAY),
      joinFirstButton(screenWidth / 2.0f + 110, screenHeight / 2.0f + 20, 200, 50, "Join First", 20,
                 BLACK, LIGHTGRAY, GRAY, DARKGRAY),
      screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      startGame_(false),
      quit_(false),
      playerName_(playerName),
      lobby_(lobby) {}

void MainMenuScene::Initialize() {
    startGame_ = false;
    quit_ = false;
    if (lobby_) {
        lobby_->RequestLobbyList(20);
    }
}

void MainMenuScene::Update() {
    quitButton.Update();
    hostButton.Update();
    refreshButton.Update();
    joinFirstButton.Update();

    if (quitButton.IsClicked()) {
        quit_ = true;
    }

    if (hostButton.IsClicked() && lobby_) {
        lobby_->HostLobby(4);
    }
    if (refreshButton.IsClicked() && lobby_) {
        lobby_->RequestLobbyList(20);
    }
    if (joinFirstButton.IsClicked() && lobby_) {
        const auto& list = lobby_->GetLobbies();
        if (!list.empty()) {
            lobby_->JoinLobby(list.front().id);
        }
    }

    if (lobby_ && lobby_->IsInLobby()) {
        startGame_ = true;
    }
}

void MainMenuScene::Draw() const {
    ClearBackground(RAYWHITE);

    const char* title = "Steam Multiplayer Game";
    int titleSize = 30;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (screenWidth_ - titleWidth) / 2, screenHeight_ / 4 - 40, titleSize, BLACK);

    // Player name label
    int nameSize = 20;
    std::string nameLine = std::string("Signed in as: ") + playerName_;
    DrawText(nameLine.c_str(), 10, 10, nameSize, DARKGRAY);

    // Verification info
    DrawText("Lobbies are tagged to this game and filtered (appid 480 safe)", 10, 35, 16, GRAY);

    hostButton.Draw();
    refreshButton.Draw();
    joinFirstButton.Draw();
    quitButton.Draw();

    // Draw lobby list
    if (lobby_) {
        int y = screenHeight_ / 2 + 90;
        DrawText("Lobbies:", 20, y, 22, BLACK);
        y += 28;
        const auto& list = lobby_->GetLobbies();
        if (list.empty()) {
            DrawText("No lobbies found.", 20, y, 18, DARKGRAY);
        } else {
            for (size_t i = 0; i < list.size() && i < 6; ++i) {
                const auto& L = list[i];
                char line[256];
                std::snprintf(line, sizeof(line), "#%zu  %s  (%d/%d)  [%s]", i + 1, L.name.c_str(), L.members, L.capacity, L.build.c_str());
                DrawText(line, 20, y + int(i) * 22, 18, BLACK);
            }
        }

        // Status / error
        std::string status = lobby_->GetStatus();
        if (!status.empty()) {
            DrawText(status.c_str(), 20, screenHeight_ - 50, 18, DARKGREEN);
        }
        if (auto err = lobby_->GetLastError()) {
            DrawText(err->c_str(), 20, screenHeight_ - 28, 18, RED);
        }
    }
}

void MainMenuScene::Unload() {}

bool MainMenuScene::ShouldStartGame() const {
    return startGame_;
}

bool MainMenuScene::ShouldQuit() const {
    return quit_;
}
