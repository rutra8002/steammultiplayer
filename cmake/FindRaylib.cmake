# FindRaylib.cmake
#
# Finds the raylib library.
#
# This module defines the following variables:
#
# RAYLIB_FOUND - True if raylib was found.
# RAYLIB_INCLUDE_DIRS - The raylib include directories.
# RAYLIB_LIBRARIES - The raylib libraries to link against.

find_path(RAYLIB_INCLUDE_DIR raylib.h
    HINTS
    /usr/local/include
    /opt/local/include
    /usr/include
    $ENV{RAYLIB_DIR}/include
    ${RAYLIB_DIR}/include
)

find_library(RAYLIB_LIBRARY
    NAMES raylib
    HINTS
    /usr/local/lib
    /opt/local/lib
    /usr/lib
    $ENV{RAYLIB_DIR}/lib
    ${RAYLIB_DIR}/lib
)

set(RAYLIB_INCLUDE_DIRS ${RAYLIB_INCLUDE_DIR})
set(RAYLIB_LIBRARIES ${RAYLIB_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Raylib DEFAULT_MSG RAYLIB_LIBRARY RAYLIB_INCLUDE_DIR)

mark_as_advanced(RAYLIB_INCLUDE_DIR RAYLIB_LIBRARY)

