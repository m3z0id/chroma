cmake_minimum_required(VERSION 4.0)
project(chroma)

set(CMAKE_EXPORT_COMPILE_COMMANDS FALSE)

set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /fp:fast")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")
endif()

add_executable(chroma
        src/main.cpp
        src/color/colorModifiers.cpp
        src/color/helpers.h
        src/Global.h)

if(WIN_32)
    target_include_directories(chroma PRIVATE ${CMAKE_SOURCE_DIR}/include/windows)
endif (WIN_32)