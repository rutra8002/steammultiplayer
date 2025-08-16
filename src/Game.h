#pragma once
#include <string>
#include <memory>
#include "scenes/Scene.h"
#include "scenes/MainMenuScene.h"
#include "scenes/GameScene.h"

enum class SceneType {
    MAIN_MENU,
    GAME
};

class Game {
public:
    Game(int width = 800, int height = 450, const char* title = "yay");
    ~Game();

    bool Init();
    void Run();
    void Shutdown();

    void ChangeScene(SceneType sceneType);

private:
    int screenWidth_;
    int screenHeight_;
    std::string title_;

    bool steamInitialized_ = false;
    std::string steamName_ = "Player";

    std::unique_ptr<Scene> currentScene_;
    SceneType currentSceneType_;
};
