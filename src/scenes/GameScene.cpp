#include "GameScene.h"
#include "raylib.h"

GameScene::GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), playerName_(playerName), net_(lobby) {}

void GameScene::Initialize() {
    // Start player in center of screen
    float px = screenWidth_ / 2.0f;
    float py = screenHeight_ / 2.0f;
    player_ = Player(px, py, 50, 50, playerName_);
}

void GameScene::Update(float deltaTime) {
    player_.Update(deltaTime);
    net_.Update(deltaTime, player_);
}

void GameScene::Draw() const {
    ClearBackground(RAYWHITE);

    // Draw local player
    player_.Draw();

    // Draw remote players
    for (const auto& [id, player] : net_.RemotePlayers()) {
        player.Draw();
    }

    DrawFPS(10, 10);
}

void GameScene::Unload() {
    net_.Clear();
}
