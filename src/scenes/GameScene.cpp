#include "GameScene.h"
#include "raylib.h"

GameScene::GameScene(int screenWidth, int screenHeight, const std::string& playerName)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), playerName_(playerName) {}

void GameScene::Initialize() {
    float px = screenWidth_ / 2.0f;
    float py = screenHeight_ / 2.0f;
    player_ = Player(px, py, 50, 50, playerName_);
}

void GameScene::Update() {
    player_.Update();
}

void GameScene::Draw() const {
    ClearBackground(RAYWHITE);
    player_.Draw();

    DrawText("Game Scene - Press ESC to return to menu", 10, 10, 20, BLACK);
}

void GameScene::Unload() {}
