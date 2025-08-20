#include "GameScene.h"
#include "raylib.h"
#include <cstring>

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

void GameScene::Update() {
    player_.Update();
    SendPlayerPosition();
    ReceivePlayerPositions();
}

void GameScene::Draw() const {
    ClearBackground(RAYWHITE);

    // Draw local player
    player_.Draw();

    // Draw remote players
    for (const auto& [id, player] : others_) {
        player.Draw();
    }

    DrawText("Game Scene - ESC leaves lobby", 10, 10, 20, BLACK);
}

void GameScene::Unload() {
    others_.clear();
}
