#pragma once

#include <vector>
#include <string>
#include <optional>
#include "steam/steam_api.h"
#include "NetConstants.h"

struct LobbyInfo {
    CSteamID id;
    std::string name;
    int members = 0;
    int capacity = 0;
    std::string build;
};

class LobbyManager {
public:
    explicit LobbyManager(const std::string& playerName);
    ~LobbyManager() = default;

    // UI helpers
    void SetPlayerName(const std::string& name);

    // Host a lobby and embed our verification tag
    void HostLobby(int maxMembers = 4);

    // Request a filtered list of lobbies that match our verification tag
    void RequestLobbyList(int maxResults = 20);

    // Join a specific lobby (will validate signature on enter and leave if invalid)
    void JoinLobby(CSteamID lobbyId);

    // Leave current lobby
    void LeaveLobby();

    // State queries
    bool IsInLobby() const { return m_inLobby; }
    CSteamID CurrentLobby() const { return m_currentLobby; }

    const std::vector<LobbyInfo>& GetLobbies() const { return m_lobbies; }
    const std::string& GetStatus() const { return m_status; }
    std::optional<std::string> GetLastError() const { return m_lastError; }

private:
    // CallResults for async Steam APIs
    CCallResult<LobbyManager, LobbyCreated_t> m_callLobbyCreated;
    CCallResult<LobbyManager, LobbyMatchList_t> m_callLobbyMatchList;
    CCallResult<LobbyManager, LobbyEnter_t> m_callLobbyEnter;

    // Callback to pick up lobby data after RequestLobbyData
    STEAM_CALLBACK(LobbyManager, OnLobbyDataUpdate, LobbyDataUpdate_t, m_cbLobbyDataUpdate);

    void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    void OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure);
    void OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure);

    // Helpers
    void SetVerificationKeys(CSteamID lobbyId);
    bool HasValidSignature(CSteamID lobbyId) const;
    LobbyInfo ReadLobbyInfo(CSteamID id) const;

private:
    std::string m_playerName;

    bool m_inLobby = false;
    CSteamID m_currentLobby;

    std::vector<LobbyInfo> m_lobbies;
    std::string m_status;
    std::optional<std::string> m_lastError;
};
