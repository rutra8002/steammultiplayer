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
    void Update(float deltaTime) override;
    void Draw() const override;
    void Unload() override;

private:
    int screenWidth_;
    int screenHeight_;
    std::string playerName_;
    Player player_;

    LobbyManager* lobby_;

    // Remote players
    std::unordered_map<uint64, Player> others_;

    // Host-authoritative movement data
    std::unordered_map<uint64, InputState> clientInputs_; // Host stores all client inputs

    // Network timing
    float networkTimer_ = 0.0f;
    float disconnectionTimer_ = 0.0f;
    float gameStateTimer_ = 0.0f;
    static constexpr float NETWORK_UPDATE_RATE = 1.0f / 60.0f; // 60 updates per second max
    static constexpr float DISCONNECTION_CHECK_RATE = 1.0f; // Check once per second
    static constexpr float GAME_STATE_SYNC_RATE = 2.0f; // Send full game state every 2 seconds

    // Host-authoritative networking methods
    void SendInputToHost();           // Client sends input to host
    void SendPositionUpdates();       // Host sends position updates to clients
    void SendGameState();             // Host sends full game state
    void ReceiveNetworkMessages();    // Process all network messages
    void UpdateAsHost(float deltaTime);    // Host game logic
    void UpdateAsClient(float deltaTime);  // Client game logic
    void RemoveDisconnectedPlayers(); // Clean up disconnected players
};
