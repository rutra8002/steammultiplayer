#include "GameScene.h"
#include "raylib.h"
#include <cstring>
#include <unordered_set>
#include "../steam/NetConstants.h"

struct NetworkMessage {
    NetConstants::MessageType type;
    uint64 playerId;
};

struct InputMessage : NetworkMessage {
    InputState input;

    InputMessage() {
        type = NetConstants::MessageType::PLAYER_INPUT;
        playerId = 0;
    }
};

struct PositionMessage : NetworkMessage {
    float x, y;

    PositionMessage() {
        type = NetConstants::MessageType::PLAYER_POSITION;
        playerId = 0;
        x = 0; y = 0;
    }
};

struct GameStateMessage : NetworkMessage {
    struct PlayerData {
        uint64 playerId;
        float x, y;
        char name[32];
    };

    uint8 playerCount;
    PlayerData players[8]; // Max 8 players

    GameStateMessage() {
        type = NetConstants::MessageType::GAME_STATE;
        playerId = 0;
        playerCount = 0;
    }
};

GameScene::GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), playerName_(playerName), lobby_(lobby) {}

void GameScene::Initialize() {
    // Start player in center of screen
    float px = static_cast<float>(screenWidth_) / 2.0f;
    float py = static_cast<float>(screenHeight_) / 2.0f;
    player_ = Player(px, py, 50, 50, playerName_);
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

    // Also clean up client inputs for disconnected players
    if (lobby_->IsHost()) {
        auto inputIt = clientInputs_.begin();
        while (inputIt != clientInputs_.end()) {
            if (currentMembers.find(inputIt->first) == currentMembers.end()) {
                inputIt = clientInputs_.erase(inputIt);
            } else {
                ++inputIt;
            }
        }
    }
}

void GameScene::Update(float deltaTime) {
    if (lobby_ && lobby_->IsHost()) {
        UpdateAsHost(deltaTime);
    } else {
        UpdateAsClient(deltaTime);
    }

    // Common networking and cleanup
    ReceiveNetworkMessages();

    disconnectionTimer_ += deltaTime;
    if (disconnectionTimer_ >= DISCONNECTION_CHECK_RATE) {
        RemoveDisconnectedPlayers();
        disconnectionTimer_ = 0.0f;
    }
}

void GameScene::UpdateAsHost(float deltaTime) {
    // Host updates all players based on their inputs

    // Update local player with direct input
    InputState myInput = player_.GetCurrentInput();
    player_.UpdateWithInput(deltaTime, myInput);

    // Update remote players with their stored inputs
    for (auto& [playerId, player] : others_) {
        auto inputIt = clientInputs_.find(playerId);
        if (inputIt != clientInputs_.end()) {
            player.UpdateWithInput(deltaTime, inputIt->second);
        }
    }

    // Send position updates to all clients
    networkTimer_ += deltaTime;
    if (networkTimer_ >= NETWORK_UPDATE_RATE) {
        SendPositionUpdates();
        networkTimer_ = 0.0f;
    }

    // Send full game state occasionally to ensure all clients are in sync
    gameStateTimer_ += deltaTime;
    if (gameStateTimer_ >= GAME_STATE_SYNC_RATE) {
        SendGameState();
        gameStateTimer_ = 0.0f;
    }
}

void GameScene::UpdateAsClient(float deltaTime) {
    // Client sends input to host and waits for position updates
    networkTimer_ += deltaTime;
    if (networkTimer_ >= NETWORK_UPDATE_RATE) {
        SendInputToHost();
        networkTimer_ = 0.0f;
    }

    // Don't update local player position - wait for host authority
    // The player position will be updated when we receive position messages
}

void GameScene::SendInputToHost() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamNetworkingMessages()) return;
    if (lobby_->IsHost()) return; // Host doesn't send input to itself

    InputMessage msg;
    msg.playerId = SteamUser()->GetSteamID().ConvertToUint64();
    msg.input = player_.GetCurrentInput();

    // Send to host
    CSteamID hostId = SteamMatchmaking()->GetLobbyOwner(lobby_->CurrentLobby());
    SteamNetworkingIdentity hostIdentity;
    hostIdentity.SetSteamID(hostId);

    SteamNetworkingMessages()->SendMessageToUser(hostIdentity, &msg, sizeof(msg),
                                                k_nSteamNetworkingSend_Unreliable, 0);
}

void GameScene::SendPositionUpdates() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamNetworkingMessages()) return;
    if (!lobby_->IsHost()) return; // Only host sends position updates

    CSteamID lobbyId = lobby_->CurrentLobby();
    int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);

    // Send position update for each player to all clients
    for (int i = 0; i < memberCount; ++i) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (member == SteamUser()->GetSteamID()) continue; // Skip self

        uint64 memberId = member.ConvertToUint64();

        // Find the player (could be local player or remote player)
        Player* playerToUpdate = nullptr;
        if (memberId == SteamUser()->GetSteamID().ConvertToUint64()) {
            playerToUpdate = &player_;
        } else {
            auto it = others_.find(memberId);
            if (it != others_.end()) {
                playerToUpdate = &it->second;
            }
        }

        if (playerToUpdate) {
            PositionMessage posMsg;
            posMsg.playerId = memberId;
            posMsg.x = playerToUpdate->GetRect().x;
            posMsg.y = playerToUpdate->GetRect().y;

            // Send to all other clients
            for (int j = 0; j < memberCount; ++j) {
                CSteamID client = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, j);
                if (client != SteamUser()->GetSteamID()) {
                    SteamNetworkingIdentity clientIdentity;
                    clientIdentity.SetSteamID(client);
                    SteamNetworkingMessages()->SendMessageToUser(clientIdentity, &posMsg, sizeof(posMsg),
                                                               k_nSteamNetworkingSend_Unreliable, 0);
                }
            }
        }
    }
}

void GameScene::SendGameState() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamNetworkingMessages()) return;
    if (!lobby_->IsHost()) return; // Only host sends game state

    GameStateMessage gameState;
    gameState.playerCount = 0;

    CSteamID lobbyId = lobby_->CurrentLobby();
    int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);

    // Add all players to game state
    for (int i = 0; i < memberCount && gameState.playerCount < 8; ++i) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        uint64 memberId = member.ConvertToUint64();

        Player* playerData = nullptr;
        if (memberId == SteamUser()->GetSteamID().ConvertToUint64()) {
            playerData = &player_;
        } else {
            auto it = others_.find(memberId);
            if (it != others_.end()) {
                playerData = &it->second;
            }
        }

        if (playerData) {
            auto& playerInfo = gameState.players[gameState.playerCount];
            playerInfo.playerId = memberId;
            playerInfo.x = playerData->GetRect().x;
            playerInfo.y = playerData->GetRect().y;

            // Get player name
            std::string playerName = "Player";
            if (memberId == SteamUser()->GetSteamID().ConvertToUint64()) {
                playerName = playerName_;
            } else if (SteamFriends()) {
                const char* steamName = SteamFriends()->GetFriendPersonaName(member);
                if (steamName && strlen(steamName) > 0) {
                    playerName = steamName;
                }
            }

            strncpy(playerInfo.name, playerName.c_str(), sizeof(playerInfo.name) - 1);
            playerInfo.name[sizeof(playerInfo.name) - 1] = '\0';

            gameState.playerCount++;
        }
    }

    // Send to all clients
    for (int i = 0; i < memberCount; ++i) {
        CSteamID client = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (client != SteamUser()->GetSteamID()) {
            SteamNetworkingIdentity clientIdentity;
            clientIdentity.SetSteamID(client);
            SteamNetworkingMessages()->SendMessageToUser(clientIdentity, &gameState, sizeof(gameState),
                                                       k_nSteamNetworkingSend_Reliable, 0);
        }
    }
}

void GameScene::ReceiveNetworkMessages() {
    if (!SteamNetworkingMessages()) return;

    ISteamNetworkingMessage* messages[20];
    int received = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, 20);

    for (int i = 0; i < received; ++i) {
        ISteamNetworkingMessage* msg = messages[i];

        // Handle different message types
        if (msg->m_cbSize >= sizeof(NetworkMessage)) {
            auto* netMsg = (NetworkMessage*)msg->m_pData;

            switch (netMsg->type) {
                case NetConstants::MessageType::PLAYER_INPUT: {
                    if (lobby_ && lobby_->IsHost() && msg->m_cbSize == sizeof(InputMessage)) {
                        auto* inputMsg = (InputMessage*)msg->m_pData;
                        uint64 senderId = msg->m_identityPeer.GetSteamID().ConvertToUint64();

                        // Store the input
                        clientInputs_[senderId] = inputMsg->input;

                        // Create player if they don't exist yet
                        if (others_.find(senderId) == others_.end()) {
                            CSteamID senderSteamID = msg->m_identityPeer.GetSteamID();
                            std::string playerName = "Remote Player";

                            if (SteamFriends()) {
                                const char* steamName = SteamFriends()->GetFriendPersonaName(senderSteamID);
                                if (steamName && strlen(steamName) > 0) {
                                    playerName = steamName;
                                }
                            }

                            // Start new players at center of screen
                            float startX = static_cast<float>(screenWidth_) / 2.0f;
                            float startY = static_cast<float>(screenHeight_) / 2.0f;
                            others_[senderId] = Player(startX, startY, 50, 50, playerName);
                        }
                    }
                    break;
                }

                case NetConstants::MessageType::PLAYER_POSITION: {
                    if (!lobby_->IsHost() && msg->m_cbSize == sizeof(PositionMessage)) {
                        auto* posMsg = (PositionMessage*)msg->m_pData;

                        if (posMsg->playerId == SteamUser()->GetSteamID().ConvertToUint64()) {
                            // Update our own position from host
                            player_.SetPosition(posMsg->x, posMsg->y);
                        } else {
                            // Update remote player position
                            auto it = others_.find(posMsg->playerId);
                            if (it != others_.end()) {
                                it->second.SetPosition(posMsg->x, posMsg->y);
                            }
                        }
                    }
                    break;
                }

                case NetConstants::MessageType::GAME_STATE: {
                    if (!lobby_->IsHost() && msg->m_cbSize == sizeof(GameStateMessage)) {
                        auto* gameState = (GameStateMessage*)msg->m_pData;

                        // Clear existing remote players
                        others_.clear();

                        // Rebuild game state
                        for (int j = 0; j < gameState->playerCount; ++j) {
                            auto& playerData = gameState->players[j];

                            if (playerData.playerId == SteamUser()->GetSteamID().ConvertToUint64()) {
                                // Update our position
                                player_.SetPosition(playerData.x, playerData.y);
                            } else {
                                // Add/update remote player
                                others_[playerData.playerId] = Player(playerData.x, playerData.y, 50, 50, playerData.name);
                            }
                        }
                    }
                    break;
                }
            }
        }

        msg->Release();
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

    // Show if we're the host
    if (lobby_ && lobby_->IsHost()) {
        DrawText("HOST", 10, 30, 20, GREEN);
    } else {
        DrawText("CLIENT", 10, 30, 20, BLUE);
    }
}

void GameScene::Unload() {
    others_.clear();
    clientInputs_.clear();
}
