#pragma once
#include <cstdint>
#include <functional>
#include <string>

#include "Modifier.h"

typedef struct s_Options {
    std::string filePath;
    std::string outputPath = "a.bmp";

    std::string modifierArg;
    std::function<void(uint8_t*, uint8_t*, uint8_t*, std::array<Modifier, 3>&)> modifierFunc = nullptr;
    ColorSpace colorSpace = ColorSpace::UNSET;

    bool printInfo = false;
    bool verbose = false;
} Options;