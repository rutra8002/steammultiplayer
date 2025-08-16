#pragma once
#include "Scene.h"
#include "../gui/Button.h"
#include <memory>

class MainMenuScene : public Scene {
public:
    MainMenuScene(int screenWidth, int screenHeight);
    ~MainMenuScene() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() const override;
    void Unload() override;

    bool ShouldStartGame() const;
    bool ShouldQuit() const;

private:
    Button playButton;
    Button quitButton;
    int screenWidth_;
    int screenHeight_;
    bool startGame_;
    bool quit_;
};
