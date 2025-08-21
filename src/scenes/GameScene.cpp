#include "GameScene.h"
#include "raylib.h"
#include <cstring>
#include <unordered_set>

// Simple message structure for player position
struct PlayerMessage {
    float x, y;
};

GameScene::GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), playerName_(playerName), lobby_(lobby) {}

void GameScene::Initialize() {
    // Start player in center of screen
    float px = screenWidth_ / 2.0f;
    float py = screenHeight_ / 2.0f;
    player_ = Player(px, py, 50, 50, playerName_);
}

void GameScene::SendPlayerPosition() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamNetworkingMessages()) return;

    PlayerMessage msg{player_.GetRect().x, player_.GetRect().y};

    // Send to all lobby members (Steam handles the routing)
    CSteamID lobbyId = lobby_->CurrentLobby();
    int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);

    for (int i = 0; i < memberCount; ++i) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (member != SteamUser()->GetSteamID()) {
            SteamNetworkingIdentity identity;
            identity.SetSteamID(member);
            SteamNetworkingMessages()->SendMessageToUser(identity, &msg, sizeof(msg),
                                                       k_nSteamNetworkingSend_Unreliable, 0);
        }
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

            uint64 senderId = msg->m_identityPeer.GetSteamID().ConvertToUint64();
            CSteamID senderSteamID = msg->m_identityPeer.GetSteamID();

            // Create or update remote player
            if (others_.find(senderId) == others_.end()) {
                // Get the player's actual Steam name
                std::string playerName = "Remote Player"; // fallback
                if (SteamFriends()) {
                    const char* steamName = SteamFriends()->GetFriendPersonaName(senderSteamID);
                    if (steamName && strlen(steamName) > 0) {
                        playerName = steamName;
                    }
                }
                others_[senderId] = Player(playerMsg.x, playerMsg.y, 50, 50, playerName);
            } else {
                others_[senderId].SetPosition(playerMsg.x, playerMsg.y);
            }
        }

        msg->Release();
    }
}

void GameScene::RemoveDisconnectedPlayers() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamMatchmaking()) return;

    // Get current lobby members
    CSteamID lobbyId = lobby_->CurrentLobby();
    int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);

    // Build set of current lobby member IDs
    std::unordered_set<uint64> currentMembers;
    for (int i = 0; i < memberCount; ++i) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (member != SteamUser()->GetSteamID()) {
            currentMembers.insert(member.ConvertToUint64());
        }
    }

    // Remove players who are no longer in the lobby
    auto it = others_.begin();
    while (it != others_.end()) {
        uint64 playerId = it->first;
        if (currentMembers.find(playerId) == currentMembers.end()) {
            it = others_.erase(it);
        } else {
            ++it;
        }
    }
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
