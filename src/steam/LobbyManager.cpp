#include "LobbyManager.h"

LobbyManager::LobbyManager(const std::string& playerName)
    : playerName_(playerName) {}

void LobbyManager::HostLobby(int maxMembers) {
    if (!SteamMatchmaking()) return;

    SteamAPICall_t hCall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxMembers);
    callLobbyCreated_.Set(hCall, this, &LobbyManager::OnLobbyCreated);
}

void LobbyManager::RequestLobbyList() {
    if (!SteamMatchmaking()) return;

    lobbies_.clear();

    // Filter lobbies to only our game using the verification tag
    SteamMatchmaking()->AddRequestLobbyListStringFilter(NetConstants::LOBBY_SIGN_KEY,
                                                        NetConstants::LOBBY_SIGN_VALUE,
                                                        k_ELobbyComparisonEqual);

    SteamAPICall_t hCall = SteamMatchmaking()->RequestLobbyList();
    callLobbyMatchList_.Set(hCall, this, &LobbyManager::OnLobbyMatchList);
}

void LobbyManager::JoinLobby(CSteamID lobbyId) {
    if (!SteamMatchmaking()) return;

    SteamAPICall_t hCall = SteamMatchmaking()->JoinLobby(lobbyId);
    callLobbyEnter_.Set(hCall, this, &LobbyManager::OnLobbyEnter);
}

void LobbyManager::LeaveLobby() {
    if (inLobby_ && SteamMatchmaking()) {
        SteamMatchmaking()->LeaveLobby(currentLobby_);
    }
    inLobby_ = false;
    currentLobby_ = CSteamID();
}

void LobbyManager::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure) {
    if (bIOFailure || pCallback->m_eResult != k_EResultOK) return;

    currentLobby_ = pCallback->m_ulSteamIDLobby;
    inLobby_ = true;

    // Set verification tag and lobby name
    SteamMatchmaking()->SetLobbyData(currentLobby_, NetConstants::LOBBY_SIGN_KEY, NetConstants::LOBBY_SIGN_VALUE);
    std::string lobbyName = playerName_ + "'s Game";
    SteamMatchmaking()->SetLobbyData(currentLobby_, "name", lobbyName.c_str());
}

void LobbyManager::OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure) {
    if (bIOFailure) return;

    lobbies_.clear();
    int32 count = pCallback->m_nLobbiesMatching;

    for (int32 i = 0; i < count; ++i) {
        CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(i);

        // Only add lobbies that belong to our game
        if (IsOurLobby(lobbyId)) {
            LobbyInfo info;
            info.id = lobbyId;
            info.name = SteamMatchmaking()->GetLobbyData(lobbyId, "name");
            info.members = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
            info.capacity = SteamMatchmaking()->GetLobbyMemberLimit(lobbyId);

            if (info.name.empty()) info.name = "Game Lobby";

            lobbies_.push_back(info);
        }
    }
}

void LobbyManager::OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure) {
    if (bIOFailure) return;

    CSteamID lobbyId = pCallback->m_ulSteamIDLobby;

    if (IsOurLobby(lobbyId)) {
        currentLobby_ = lobbyId;
        inLobby_ = true;
    } else {
        SteamMatchmaking()->LeaveLobby(lobbyId);
    }
}

bool LobbyManager::IsOurLobby(CSteamID lobbyId) const {
    if (!SteamMatchmaking()) return false;

    const char* lobbyTag = SteamMatchmaking()->GetLobbyData(lobbyId, NetConstants::LOBBY_SIGN_KEY);
    return lobbyTag && std::string(lobbyTag) == NetConstants::LOBBY_SIGN_VALUE;
}
