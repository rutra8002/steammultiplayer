#pragma once

// Unique lobby verification tag to distinguish this game's lobbies from other Spacewar (appid 480) lobbies.
// Change this to any unique string for your project. Keep both key and value stable across builds.
namespace NetConstants {
    inline constexpr const char* LOBBY_SIGN_KEY = "app_tag"; // metadata key
    inline constexpr const char* LOBBY_SIGN_VALUE = "ruterek"; // unique value
    inline constexpr const char* LOBBY_NAME_KEY = "name"; // display title in list
    inline constexpr const char* LOBBY_BUILD_KEY = "build"; // optional build marker
}

