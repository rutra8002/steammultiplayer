#include <iostream>
#include "steam/steam_api.h"

int main() {
    if (SteamAPI_Init()) {
        std::cout << "Steam API initialized successfully!" << std::endl;

        const char *steamName = SteamFriends()->GetPersonaName();
        std::cout << "name: " << steamName << std::endl;

        int steamLevel = SteamUser()->GetPlayerSteamLevel();
        std::cout << "steam level: " << steamLevel << std::endl;

    } else {
        std::cerr << "Failed to initialize Steam API!" << std::endl;
        return 1;
    }

    SteamAPI_Shutdown();
    return 0;
}