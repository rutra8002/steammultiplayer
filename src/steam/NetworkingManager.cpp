#include "NetworkingManager.h"
#include <cstring>
#include <unordered_set>
#include <algorithm>

struct PlayerMessage {
    float x, y;
};

NetworkingManager::NetworkingManager(LobbyManager* lobby) : lobby_(lobby) {}

std::string NetworkingManager::GetSteamPlayerName(CSteamID steamID) {
    if (!SteamFriends()) return "Remote Player";

    const char* steamName = SteamFriends()->GetFriendPersonaName(steamID);
    return (steamName && strlen(steamName) > 0) ? steamName : "Remote Player";
}

bool NetworkingManager::IsSteamNetworkingAvailable() const {
    return lobby_ && lobby_->IsInLobby() && SteamNetworkingMessages();
}

std::vector<CSteamID> NetworkingManager::GetOtherLobbyMembers() const {
    std::vector<CSteamID> members;
    if (!lobby_ || !lobby_->IsInLobby() || !SteamMatchmaking()) return members;

    CSteamID lobbyId = lobby_->CurrentLobby();
    int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);

    for (int i = 0; i < memberCount; ++i) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (SteamUser() && member != SteamUser()->GetSteamID()) {
            members.push_back(member);
        }
    }
    return members;
}

void NetworkingManager::SendPlayerPosition(const Player& localPlayer) {
    if (!IsSteamNetworkingAvailable()) return;

    PlayerMessage msg{localPlayer.GetRect().x, localPlayer.GetRect().y};

    for (const CSteamID& member : GetOtherLobbyMembers()) {
        SteamNetworkingIdentity identity;
        identity.SetSteamID(member);
        SteamNetworkingMessages()->SendMessageToUser(
            identity, &msg, sizeof(msg), k_nSteamNetworkingSend_Unreliable, 0);
    }
}

void NetworkingManager::ReceivePlayerPositions() {
    if (!SteamNetworkingMessages()) return;

    ISteamNetworkingMessage* messages[10];
    int received = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, 10);

    for (int i = 0; i < received; ++i) {
        ISteamNetworkingMessage* msg = messages[i];

        if (msg->m_cbSize == sizeof(PlayerMessage)) {
            PlayerMessage playerMsg{};
            memcpy(&playerMsg, msg->m_pData, sizeof(PlayerMessage));

            CSteamID senderSteamID = msg->m_identityPeer.GetSteamID();
            uint64_t senderId = senderSteamID.ConvertToUint64();

            // Create or update remote player
            auto it = others_.find(senderId);
            if (it == others_.end()) {
                others_[senderId] = Player(playerMsg.x, playerMsg.y, 50, 50, GetSteamPlayerName(senderSteamID));
            } else {
                it->second.SetPosition(playerMsg.x, playerMsg.y);
            }
        }

        msg->Release();
    }
}

void NetworkingManager::RemoveDisconnectedPlayers() {
    if (!IsSteamNetworkingAvailable()) return;

    // Get current lobby member IDs
    std::unordered_set<uint64_t> currentMembers;
    for (const CSteamID& member : GetOtherLobbyMembers()) {
        currentMembers.insert(member.ConvertToUint64());
    }

    // Remove players who are no longer in the lobby
    std::erase_if(others_, [&currentMembers](const auto& pair) {
        return currentMembers.find(pair.first) == currentMembers.end();
    });
}

void NetworkingManager::Clear() {
    others_.clear();
    networkTimer_ = 0.0f;
    disconnectionTimer_ = 0.0f;
}

void NetworkingManager::Update(float deltaTime, const Player& localPlayer) {
    // Receive first for responsiveness
    ReceivePlayerPositions();

    networkTimer_ += deltaTime;
    if (networkTimer_ >= NETWORK_UPDATE_RATE) {
        SendPlayerPosition(localPlayer);
        networkTimer_ = 0.0f;
    }

    disconnectionTimer_ += deltaTime;
    if (disconnectionTimer_ >= DISCONNECTION_CHECK_RATE) {
        RemoveDisconnectedPlayers();
        disconnectionTimer_ = 0.0f;
    }
}

