#include "MainMenuScene.h"
#include "raylib.h"

MainMenuScene::MainMenuScene(int screenWidth, int screenHeight)
    : playButton(screenWidth / 2.0f - 100, screenHeight / 2.0f - 25, 200, 50, "Play Game", 20,
                 BLACK, LIGHTGRAY, GRAY, DARKGRAY),
      quitButton(screenWidth / 2.0f - 100, screenHeight / 2.0f + 50, 200, 50, "Quit Game", 20,
                 BLACK, LIGHTGRAY, GRAY, DARKGRAY),
      screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      startGame_(false),
      quit_(false) {}

void MainMenuScene::Initialize() {
    startGame_ = false;
    quit_ = false;
}

void MainMenuScene::Update() {
    playButton.Update();
    quitButton.Update();

    if (playButton.IsClicked()) {
        startGame_ = true;
    }

    if (quitButton.IsClicked()) {
        quit_ = true;
    }
}

void MainMenuScene::Draw() const {
    ClearBackground(RAYWHITE);

    const char* title = "Steam Multiplayer Game";
    int titleSize = 30;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (screenWidth_ - titleWidth) / 2, screenHeight_ / 4, titleSize, BLACK);

    playButton.Draw();
    quitButton.Draw();
}

void MainMenuScene::Unload() {}

bool MainMenuScene::ShouldStartGame() const {
    return startGame_;
}

bool MainMenuScene::ShouldQuit() const {
    return quit_;
}
