cmake_minimum_required(VERSION 4.0)
project(chroma)

# Disable VS Code shit
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS FALSE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

add_executable(chroma
        src/main.cpp)

