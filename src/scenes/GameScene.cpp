#include "GameScene.h"
#include "raylib.h"
#include <cstring>
#include <unordered_set>
#include <algorithm>
#include <vector>

// Simple message structure for player position
struct PlayerMessage {
    float x, y;
};

std::string GetSteamPlayerName(CSteamID steamID) {
    if (!SteamFriends()) return "Remote Player";

    const char* steamName = SteamFriends()->GetFriendPersonaName(steamID);
    return (steamName && strlen(steamName) > 0) ? steamName : "Remote Player";
}

bool IsSteamNetworkingAvailable(LobbyManager* lobby) {
    return lobby && lobby->IsInLobby() && SteamNetworkingMessages();
}

std::vector<CSteamID> GetOtherLobbyMembers(LobbyManager* lobby) {
    std::vector<CSteamID> members;
    if (!lobby || !lobby->IsInLobby() || !SteamMatchmaking()) return members;

    CSteamID lobbyId = lobby->CurrentLobby();
    int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);

    for (int i = 0; i < memberCount; ++i) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (member != SteamUser()->GetSteamID()) {
            members.push_back(member);
        }
    }
    return members;
}

GameScene::GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), playerName_(playerName), lobby_(lobby) {}

void GameScene::Initialize() {
    // Start player in center of screen
    float px = screenWidth_ / 2.0f;
    float py = screenHeight_ / 2.0f;
    player_ = Player(px, py, 50, 50, playerName_);
}

void GameScene::SendPlayerPosition() {
    if (!IsSteamNetworkingAvailable(lobby_)) return;

    PlayerMessage msg{player_.GetRect().x, player_.GetRect().y};

    for (const CSteamID& member : GetOtherLobbyMembers(lobby_)) {
        SteamNetworkingIdentity identity;
        identity.SetSteamID(member);
        SteamNetworkingMessages()->SendMessageToUser(identity, &msg, sizeof(msg),
                                                   k_nSteamNetworkingSend_Unreliable, 0);
    }
}

void GameScene::ReceivePlayerPositions() {
    if (!SteamNetworkingMessages()) return;

    ISteamNetworkingMessage* messages[10];
    int received = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, 10);

    for (int i = 0; i < received; ++i) {
        ISteamNetworkingMessage* msg = messages[i];

        if (msg->m_cbSize == sizeof(PlayerMessage)) {
            PlayerMessage playerMsg;
            memcpy(&playerMsg, msg->m_pData, sizeof(PlayerMessage));

            CSteamID senderSteamID = msg->m_identityPeer.GetSteamID();
            uint64 senderId = senderSteamID.ConvertToUint64();

            // Create or update remote player
            if (others_.find(senderId) == others_.end()) {
                others_[senderId] = Player(playerMsg.x, playerMsg.y, 50, 50, GetSteamPlayerName(senderSteamID));
            } else {
                others_[senderId].SetPosition(playerMsg.x, playerMsg.y);
            }
        }

        msg->Release();
    }
}

void GameScene::RemoveDisconnectedPlayers() {
    if (!IsSteamNetworkingAvailable(lobby_)) return;

    // Get current lobby member IDs
    std::unordered_set<uint64> currentMembers;
    for (const CSteamID& member : GetOtherLobbyMembers(lobby_)) {
        currentMembers.insert(member.ConvertToUint64());
    }

    // Remove players who are no longer in the lobby
    std::erase_if(others_, [&currentMembers](const auto& pair) {
        return currentMembers.find(pair.first) == currentMembers.end();
    });
}

void GameScene::Update(float deltaTime) {
    player_.Update(deltaTime);

    ReceivePlayerPositions();

    networkTimer_ += deltaTime;
    if (networkTimer_ >= NETWORK_UPDATE_RATE) {
        SendPlayerPosition();
        networkTimer_ = 0.0f;
    }

    disconnectionTimer_ += deltaTime;
    if (disconnectionTimer_ >= DISCONNECTION_CHECK_RATE) {
        RemoveDisconnectedPlayers();
        disconnectionTimer_ = 0.0f;
    }
}

void GameScene::Draw() const {
    ClearBackground(RAYWHITE);

    // Draw local player
    player_.Draw();

    // Draw remote players
    for (const auto& [id, player] : others_) {
        player.Draw();
    }

    DrawFPS(10, 10);
}

void GameScene::Unload() {
    others_.clear();
}
