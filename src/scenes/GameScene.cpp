#include "GameScene.h"
#include "raylib.h"
#include <vector>
#include <cstring>

namespace {
struct MsgPos {
    float x;
    float y;
};
}

GameScene::GameScene(int screenWidth, int screenHeight, const std::string& playerName, LobbyManager* lobby)
    : screenWidth_(screenWidth), screenHeight_(screenHeight), playerName_(playerName), lobby_(lobby),
      cbSessionRequest_(this, &GameScene::OnMsgSessionRequest) {}

void GameScene::Initialize() {
    float px = screenWidth_ / 2.0f;
    float py = screenHeight_ / 2.0f;
    player_ = Player(px, py, 50, 50, playerName_);

    localId_ = SteamUser() ? SteamUser()->GetSteamID() : CSteamID();
    RefreshLobbyMembers();
}

void GameScene::RefreshLobbyMembers() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamMatchmaking()) return;

    CSteamID lobbyId = lobby_->CurrentLobby();
    int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
    for (int i = 0; i < count; ++i) {
        CSteamID m = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (m == localId_) continue;
        uint64 id64 = m.ConvertToUint64();
        if (others_.find(id64) == others_.end()) {
            // Place new remote players near center offset by index
            float ox = screenWidth_ / 2.0f + (i * 30.0f);
            float oy = screenHeight_ / 2.0f + (i * 20.0f);
            const char* pname = SteamFriends() ? SteamFriends()->GetFriendPersonaName(m) : "Remote";
            Player p(ox, oy, 50, 50, pname ? std::string(pname) : std::string("Remote"));
            others_.emplace(id64, p);
        }
    }
}

void GameScene::SendMyState() {
    if (!lobby_ || !lobby_->IsInLobby() || !SteamMatchmaking() || !SteamNetworkingMessages()) return;
    CSteamID lobbyId = lobby_->CurrentLobby();
    int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
    MsgPos msg{ player_.GetRect().x, player_.GetRect().y };
    for (int i = 0; i < count; ++i) {
        CSteamID m = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i);
        if (m == localId_) continue;
        SteamNetworkingIdentity id;
        id.SetSteamID(m);
        SteamNetworkingMessages()->SendMessageToUser(id, &msg, sizeof(msg), k_nSteamNetworkingSend_Unreliable, 0);
    }
}

void GameScene::ReceiveMessages() {
    if (!SteamNetworkingMessages()) return;
    ISteamNetworkingMessage* msgs[16] = {};
    int received = 0;
    while ((received = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, msgs, 16)) > 0) {
        for (int i = 0; i < received; ++i) {
            ISteamNetworkingMessage* m = msgs[i];
            // Identify peer
            CSteamID peer = m->m_identityPeer.GetSteamID();
            // Validate sender is in our lobby
            bool valid = peer.IsValid();
            if (valid && lobby_ && lobby_->IsInLobby() && SteamMatchmaking()) {
                CSteamID lobbyId = lobby_->CurrentLobby();
                int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
                valid = false;
                for (int j = 0; j < count; ++j) {
                    if (SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, j) == peer) { valid = true; break; }
                }
            }
            if (valid && m->m_cbSize == sizeof(MsgPos)) {
                MsgPos pos;
                std::memcpy(&pos, m->m_pData, sizeof(MsgPos));
                uint64 id64 = peer.ConvertToUint64();
                auto it = others_.find(id64);
                if (it == others_.end()) {
                    const char* pname = SteamFriends() ? SteamFriends()->GetFriendPersonaName(peer) : "Remote";
                    Player p(pos.x, pos.y, 50, 50, pname ? std::string(pname) : std::string("Remote"));
                    others_.emplace(id64, p);
                } else {
                    it->second.SetPosition(pos.x, pos.y);
                }
            }
            m->Release();
        }
    }
}

void GameScene::OnMsgSessionRequest(SteamNetworkingMessagesSessionRequest_t* pReq) {
    // Accept sessions from lobby members only
    if (!lobby_ || !lobby_->IsInLobby() || !SteamNetworkingMessages() || !SteamMatchmaking()) return;
    CSteamID from = pReq->m_identityRemote.GetSteamID();
    bool ok = from.IsValid();
    if (ok) {
        CSteamID lobbyId = lobby_->CurrentLobby();
        int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
        ok = false;
        for (int i = 0; i < count; ++i) {
            if (SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i) == from) { ok = true; break; }
        }
    }
    if (ok) {
        SteamNetworkingMessages()->AcceptSessionWithUser(pReq->m_identityRemote);
    }
}

void GameScene::Update() {
    player_.Update();

    // Ensure list is up to date
    RefreshLobbyMembers();

    // Network
    SendMyState();
    ReceiveMessages();
}

void GameScene::Draw() const {
    ClearBackground(RAYWHITE);
    // Draw local and remote players
    player_.Draw();
    for (const auto& kv : others_) {
        kv.second.Draw();
    }

    DrawText("Game Scene - ESC leaves lobby", 10, 10, 20, BLACK);
}

void GameScene::Unload() {}
