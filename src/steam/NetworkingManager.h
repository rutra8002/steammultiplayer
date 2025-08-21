#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include "steam/steam_api.h"
#include "LobbyManager.h"
#include "../Player.h"

class NetworkingManager {
public:
    explicit NetworkingManager(LobbyManager* lobby);
    ~NetworkingManager() = default;

    // Drive networking: poll incoming, send local, prune disconnected
    void Update(float deltaTime, const Player& localPlayer);

    const std::unordered_map<uint64_t, Player>& RemotePlayers() const { return others_; }
    void Clear();

private:
    LobbyManager* lobby_ { nullptr };

    // Remote players
    std::unordered_map<uint64_t, Player> others_;

    // Network timing
    float networkTimer_ = 0.0f;
    float disconnectionTimer_ = 0.0f;
    static constexpr float NETWORK_UPDATE_RATE = 1.0f / 60.0f; // 60 updates per second max
    static constexpr float DISCONNECTION_CHECK_RATE = 1.0f; // Check once per second

private:
    void SendPlayerPosition(const Player& localPlayer);
    void ReceivePlayerPositions();
    void RemoveDisconnectedPlayers();

    // Helpers
    static std::string GetSteamPlayerName(CSteamID steamID);
    bool IsSteamNetworkingAvailable() const;
    std::vector<CSteamID> GetOtherLobbyMembers() const;
};

