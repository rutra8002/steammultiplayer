#pragma once
#include "Scene.h"
#include "../gui/Button.h"
#include <memory>
#include <string>
#include "../steam/LobbyManager.h"

class MainMenuScene : public Scene {
public:
    MainMenuScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby);
    ~MainMenuScene() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void Unload() override;

    bool ShouldStartGame() const;
    bool ShouldQuit() const;

private:
    Button quitButton;
    Button hostButton;
    Button refreshButton;
    Button joinFirstButton;
    int screenWidth_;
    int screenHeight_;
    bool startGame_;
    bool quit_;

    std::string playerName_;
    LobbyManager* lobby_;
};
