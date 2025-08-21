#pragma once

// Unique lobby verification tag to distinguish this game's lobbies from other Spacewar (appid 480) lobbies.
namespace NetConstants {
    inline constexpr const char* LOBBY_SIGN_KEY = "app_tag"; // metadata key
    inline constexpr const char* LOBBY_SIGN_VALUE = "ruterek"; // unique value
    inline constexpr const char* LOBBY_NAME_KEY = "name"; // display title in list

    // Message types for host-authoritative movement
    enum class MessageType : uint8_t {
        PLAYER_INPUT = 1,        // Client sends input to host
        PLAYER_POSITION = 2,     // Host sends authoritative position updates
        GAME_STATE = 3          // Full game state sync
    };
}
