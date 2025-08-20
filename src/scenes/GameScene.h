#pragma once
#include "Scene.h"
#include "../Player.h"
#include <string>
#include <unordered_map>
#include "steam/steam_api.h"
#include "../steam/LobbyManager.h"

class GameScene : public Scene {
public:
    GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby);
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

    LobbyManager* lobby_;

    // Simple remote players map
    std::unordered_map<uint64, Player> others_;

    void SendPlayerPosition();
    void ReceivePlayerPositions();
};
