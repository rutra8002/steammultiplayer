# Find Steamworks SDK
#
# This module can be used to find the Steamworks SDK.
#
# It sets the following variables:
#   STEAMWORKS_FOUND - True if the SDK was found.
#   STEAMWORKS_INCLUDE_DIRS - The include directories of the SDK.
#   STEAMWORKS_LIBRARIES - The libraries of the SDK.

find_path(STEAMWORKS_INCLUDE_DIRS
        NAMES steam/steam_api.h
        HINTS
            ENV STEAM_SDK_LOCATION
            "${CMAKE_SOURCE_DIR}"
        PATH_SUFFIXES steamworks_sdk/public)

find_library(STEAMWORKS_LIBRARIES
        NAMES steam_api
        HINTS
            ENV STEAM_SDK_LOCATION
            "${CMAKE_SOURCE_DIR}"
        PATH_SUFFIXES steamworks_sdk/redistributable_bin/linux64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Steamworks DEFAULT_MSG STEAMWORKS_LIBRARIES STEAMWORKS_INCLUDE_DIRS)

mark_as_advanced(STEAMWORKS_INCLUDE_DIRS STEAMWORKS_LIBRARIES)
