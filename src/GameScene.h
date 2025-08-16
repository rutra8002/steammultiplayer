#pragma once
#include "Scene.h"
#include "Player.h"
#include <string>

class GameScene : public Scene {
public:
    GameScene(int screenWidth, int screenHeight, const std::string& playerName);
    ~GameScene() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() const override;
    void Unload() override;

private:
    int screenWidth_;
    int screenHeight_;
    std::string playerName_;
    Player player_;
};
