#include <iostream>
#include "steam/steam_api.h"
#include "raylib.h"

int main() {
    if (SteamAPI_Init()) {
        std::cout << "Steam API initialized successfully!" << std::endl;

    } else {
        std::cerr << "Failed to initialize Steam API!" << std::endl;
        return 1;
    }

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "yay");

    const char *steamName = SteamFriends()->GetPersonaName();

    Rectangle player = { (float)screenWidth / 2, (float)screenHeight / 2, 50, 50 };

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) player.x += 5.0f;
        if (IsKeyDown(KEY_LEFT)) player.x -= 5.0f;
        if (IsKeyDown(KEY_UP)) player.y -= 5.0f;
        if (IsKeyDown(KEY_DOWN)) player.y += 5.0f;

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawRectangleRec(player, RED);
        int textWidth = MeasureText(steamName, 20);
        DrawText(steamName, player.x + player.width / 2 - textWidth / 2, player.y - 25, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();

    SteamAPI_Shutdown();
    return 0;
}