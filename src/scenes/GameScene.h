#pragma once
#include "Scene.h"
#include "../Player.h"
#include <string>
#include "../steam/NetworkingManager.h"

class LobbyManager;

class GameScene : public Scene {
public:
    GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby);
    ~GameScene() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void Unload() override;

private:
    int screenWidth_;
    int screenHeight_;
    std::string playerName_;
    Player player_;

    NetworkingManager net_;
};
