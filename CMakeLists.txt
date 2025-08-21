cmake_minimum_required(VERSION 3.10)
project(steammultiplayer)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(STEAM_SDK_LOCATION ${CMAKE_SOURCE_DIR}/steamworks_sdk)
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib)
find_package(Steamworks REQUIRED)
find_package(Raylib REQUIRED)

add_executable(steammultiplayer
    main.cpp
    src/Game.cpp
    src/Player.cpp
    src/gui/Button.cpp
    src/scenes/MainMenuScene.cpp
    src/scenes/GameScene.cpp
    src/steam/LobbyManager.cpp
    src/steam/NetworkingManager.cpp
)

file(COPY ${CMAKE_SOURCE_DIR}/steam_appid.txt DESTINATION ${CMAKE_BINARY_DIR})

target_include_directories(steammultiplayer PRIVATE ${STEAMWORKS_INCLUDE_DIRS} ${RAYLIB_INCLUDE_DIRS})
target_link_libraries(steammultiplayer PRIVATE ${STEAMWORKS_LIBRARIES} ${RAYLIB_LIBRARIES})
