#include <iostream>
#include "steam/steam_api.h"

int main() {
    if (SteamAPI_Init()) {
        std::cout << "Steam API initialized successfully!" << std::endl;
    } else {
        std::cerr << "Failed to initialize Steam API!" << std::endl;
        return 1;
    }

    std::cout << "Hello, World!" << std::endl;

    SteamAPI_Shutdown();
    return 0;
}