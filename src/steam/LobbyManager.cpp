#include "LobbyManager.h"
#include <algorithm>
#include <cstdio>

LobbyManager::LobbyManager(const std::string& playerName)
    : m_playerName(playerName),
      m_cbLobbyDataUpdate(this, &LobbyManager::OnLobbyDataUpdate) {}

void LobbyManager::SetPlayerName(const std::string& name) {
    m_playerName = name;
}

void LobbyManager::HostLobby(int maxMembers) {
    if (!SteamMatchmaking()) {
        m_lastError = "SteamMatchmaking not available";
        return;
    }
    m_status = "Creating lobby...";
    SteamAPICall_t hCall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxMembers);
    m_callLobbyCreated.Set(hCall, this, &LobbyManager::OnLobbyCreated);
}

void LobbyManager::RequestLobbyList(int maxResults) {
    if (!SteamMatchmaking()) {
        m_lastError = "SteamMatchmaking not available";
        return;
    }
    m_lobbies.clear();
    m_status = "Requesting lobby list...";

    SteamMatchmaking()->AddRequestLobbyListStringFilter(NetConstants::LOBBY_SIGN_KEY,
                                                        NetConstants::LOBBY_SIGN_VALUE,
                                                        k_ELobbyComparisonEqual);
    if (maxResults > 0) {
        SteamMatchmaking()->AddRequestLobbyListResultCountFilter(maxResults);
    }

    SteamAPICall_t hCall = SteamMatchmaking()->RequestLobbyList();
    m_callLobbyMatchList.Set(hCall, this, &LobbyManager::OnLobbyMatchList);
}

void LobbyManager::JoinLobby(CSteamID lobbyId) {
    if (!SteamMatchmaking()) {
        m_lastError = "SteamMatchmaking not available";
        return;
    }
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Joining lobby %llu...", lobbyId.ConvertToUint64());
    m_status = buf;

    SteamAPICall_t hCall = SteamMatchmaking()->JoinLobby(lobbyId);
    m_callLobbyEnter.Set(hCall, this, &LobbyManager::OnLobbyEnter);
}

void LobbyManager::LeaveLobby() {
    if (m_inLobby && SteamMatchmaking()) {
        SteamMatchmaking()->LeaveLobby(m_currentLobby);
    }
    m_inLobby = false;
    m_currentLobby = CSteamID();
}

void LobbyManager::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure) {
    if (bIOFailure || pCallback->m_eResult != k_EResultOK) {
        m_lastError = "Lobby creation failed";
        m_status.clear();
        return;
    }

    m_currentLobby = pCallback->m_ulSteamIDLobby;
    m_inLobby = true;

    // Set verification and display metadata
    SetVerificationKeys(m_currentLobby);

    // A friendly name for UI
    std::string lobbyName = m_playerName.empty() ? "Host" : (m_playerName + "'s Lobby");
    SteamMatchmaking()->SetLobbyData(m_currentLobby, NetConstants::LOBBY_NAME_KEY, lobbyName.c_str());

    m_status = "Lobby created";
}

void LobbyManager::OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure) {
    if (bIOFailure) {
        m_lastError = "Lobby list IO failure";
        m_status.clear();
        return;
    }

    m_lobbies.clear();
    int32 count = pCallback->m_nLobbiesMatching;
    for (int32 i = 0; i < count; ++i) {
        CSteamID id = SteamMatchmaking()->GetLobbyByIndex(i);
        // Ask Steam to send us the data; we'll also try to read immediately.
        SteamMatchmaking()->RequestLobbyData(id);
        if (HasValidSignature(id)) {
            m_lobbies.push_back(ReadLobbyInfo(id));
        }
    }

    m_status = "Found " + std::to_string(m_lobbies.size()) + " lobby(ies)";
}

void LobbyManager::OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure) {
    if (bIOFailure) {
        m_lastError = "Join lobby IO failure";
        m_status.clear();
        return;
    }

    CSteamID lobbyId = pCallback->m_ulSteamIDLobby;
    if (!HasValidSignature(lobbyId)) {
        // Leave immediately if signature doesn't match
        SteamMatchmaking()->LeaveLobby(lobbyId);
        m_inLobby = false;
        m_currentLobby = CSteamID();
        m_lastError = "Rejected lobby: invalid game signature";
        m_status.clear();
        return;
    }

    m_currentLobby = lobbyId;
    m_inLobby = true;

    m_status = "Joined lobby";
}

void LobbyManager::OnLobbyDataUpdate(LobbyDataUpdate_t* pCallback) {
    if (!pCallback->m_bSuccess) return;

    CSteamID lobbyId = pCallback->m_ulSteamIDLobby;
    // Only care about lobby data (not individual members)
    if (pCallback->m_ulSteamIDLobby != pCallback->m_ulSteamIDMember) {
        return;
    }

    // Update or insert entry if it has a valid signature
    auto it = std::find_if(m_lobbies.begin(), m_lobbies.end(), [&](const LobbyInfo& li){ return li.id == lobbyId; });
    if (HasValidSignature(lobbyId)) {
        LobbyInfo info = ReadLobbyInfo(lobbyId);
        if (it == m_lobbies.end()) {
            m_lobbies.push_back(info);
        } else {
            *it = info;
        }
    } else {
        if (it != m_lobbies.end()) m_lobbies.erase(it);
    }
}

void LobbyManager::SetVerificationKeys(CSteamID lobbyId) {
    if (!SteamMatchmaking()) return;
    SteamMatchmaking()->SetLobbyData(lobbyId, NetConstants::LOBBY_SIGN_KEY, NetConstants::LOBBY_SIGN_VALUE);
#ifdef NDEBUG
    const char* build = "release";
#else
    const char* build = "debug";
#endif
    SteamMatchmaking()->SetLobbyData(lobbyId, NetConstants::LOBBY_BUILD_KEY, build);
}

bool LobbyManager::HasValidSignature(CSteamID lobbyId) const {
    if (!SteamMatchmaking()) return false;
    const char* v = SteamMatchmaking()->GetLobbyData(lobbyId, NetConstants::LOBBY_SIGN_KEY);
    return v && std::string(v) == NetConstants::LOBBY_SIGN_VALUE;
}

LobbyInfo LobbyManager::ReadLobbyInfo(CSteamID id) const {
    LobbyInfo li{};
    li.id = id;
    if (SteamMatchmaking()) {
        const char* name = SteamMatchmaking()->GetLobbyData(id, NetConstants::LOBBY_NAME_KEY);
        if (!name || !*name) name = SteamFriends() ? SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyOwner(id)) : "Lobby";
        li.name = name ? name : "Lobby";
        li.members = SteamMatchmaking()->GetNumLobbyMembers(id);
        li.capacity = SteamMatchmaking()->GetLobbyMemberLimit(id);
        const char* build = SteamMatchmaking()->GetLobbyData(id, NetConstants::LOBBY_BUILD_KEY);
        li.build = build ? build : "";
    }
    return li;
}
