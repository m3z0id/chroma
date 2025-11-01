cmake_minimum_required(VERSION 3.15)
project(chroma)
include(FetchContent)

# Enable compile commands for tooling support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /fp:fast")
else()
    add_compile_options(-Wall -Wextra)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")
endif()

# Source files
add_executable(chroma
        src/main.cpp
        src/color/colorModifiers.cpp
        src/color/helpers.h
        src/Global.h
)

# Windows-specific includes
if(WIN32)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library (produces getopt.dll and getopt.lib)")
    set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static library (produces getopt_static.lib)")
    set(BUILD_TESTING OFF CACHE BOOL "Enable tests for both shared and static libraries")
    FetchContent_Declare(
            getopt_win
            GIT_REPOSITORY https://github.com/ludvikjerabek/getopt-win.git
            GIT_TAG        v2.42.0
    )
    FetchContent_MakeAvailable(getopt_win)
    target_link_libraries(myprogram PRIVATE getopt::getopt_static)
    target_compile_definitions(myprogram PRIVATE STATIC_GETOPT)
    target_include_directories(chroma PRIVATE ${getopt_win_SOURCE_DIR})
endif()
