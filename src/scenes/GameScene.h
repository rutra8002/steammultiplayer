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
    CSteamID localId_;

    // steam networking (messages)
    STEAM_CALLBACK(GameScene, OnMsgSessionRequest, SteamNetworkingMessagesSessionRequest_t, cbSessionRequest_);

    // remote players by steamID64
    std::unordered_map<uint64, Player> others_;

    void RefreshLobbyMembers();
    void SendMyState();
    void ReceiveMessages();
};
