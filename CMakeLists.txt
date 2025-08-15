cmake_minimum_required(VERSION 3.10)
project(steammultiplayer)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Steamworks REQUIRED)

add_executable(steammultiplayer main.cpp)

file(COPY ${CMAKE_SOURCE_DIR}/steam_appid.txt DESTINATION ${CMAKE_BINARY_DIR})

target_include_directories(steammultiplayer PRIVATE ${STEAMWORKS_INCLUDE_DIRS})
target_link_libraries(steammultiplayer PRIVATE ${STEAMWORKS_LIBRARIES})
