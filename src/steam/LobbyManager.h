#pragma once

#include <vector>
#include <string>
#include "steam/steam_api.h"
#include "NetConstants.h"

struct LobbyInfo {
    CSteamID id;
    std::string name;
    int members = 0;
    int capacity = 0;
};

class LobbyManager {
public:
    explicit LobbyManager(const std::string& playerName);
    ~LobbyManager() = default;

    void SetPlayerName(const std::string& name) { playerName_ = name; }

    // Simple lobby operations with verification
    void HostLobby(int maxMembers = 4);
    void RequestLobbyList();
    void JoinLobby(CSteamID lobbyId);
    void LeaveLobby();

    // State queries
    bool IsInLobby() const { return inLobby_; }
    CSteamID CurrentLobby() const { return currentLobby_; }
    const std::vector<LobbyInfo>& GetLobbies() const { return lobbies_; }

private:
    // Simple callbacks for async Steam APIs
    CCallResult<LobbyManager, LobbyCreated_t> callLobbyCreated_;
    CCallResult<LobbyManager, LobbyMatchList_t> callLobbyMatchList_;
    CCallResult<LobbyManager, LobbyEnter_t> callLobbyEnter_;

    void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    void OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure);
    void OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure);

    // Simple verification helper
    bool IsOurLobby(CSteamID lobbyId) const;

private:
    std::string playerName_;
    bool inLobby_ = false;
    CSteamID currentLobby_;
    std::vector<LobbyInfo> lobbies_;
};
