#pragma once
#include "../Global.h"
#include "../color/colorModifiers.h"
#include "../datatypes/Options.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <sstream>

const std::array<std::string, 4> VALID_CHANNELS_MAP = {"RGBrgb", "HSLhsl", "LABlab", "LCHlch"};

inline Modifier parseModifierArg(const std::string& arg, const ColorSpace colorSpace, const char channel) {
    Modifier mod{};
    uint8_t numberParseStartIndex = 0;

    switch(arg.at(0)) {
        case '~':
            numberParseStartIndex = 1;
            mod.operation = Operation::INVERT;
            break;
        case '+':
        case '-':
            mod.operation = Operation::ADD;
            break;
        default:
            if (isdigit(arg.at(0)) || VALID_CHANNELS_MAP.at(getColorSpaceIndex(colorSpace)).contains(arg)) mod.operation = Operation::SET;
            else {
                std::cerr << "Error while parsing channel argument value\n";
                return {};
            }
    }
    mod.editChannel = channel;

    std::string numberStr = arg.substr(numberParseStartIndex);
    const char numberChar = numberStr.at(0);
    if (!isdigit(numberChar) && numberChar != '+' && numberChar != '-') {
        if (VALID_CHANNELS_MAP.at(getColorSpaceIndex(colorSpace)).contains(numberChar)) {
            mod.modifierChannel = numberChar;
            if (arg.at(0) == '-') mod.modifierChannel = -mod.modifierChannel;
        }
        return mod;
    }

    uint8_t idx = 0;
    char current;
    bool seenDecimalPoint = false;
    while (idx < numberStr.size() && (isdigit(current = numberStr.at(idx)) || current == '.' || current == ',' || current == '+' || current == '-')) {
        if (!seenDecimalPoint && (current == '.' || current == ',')) {
            seenDecimalPoint = true;
            if (current == ',') numberStr[idx] = '.';
        }
        idx++;
    }

    mod.difference = std::stof(numberStr.substr(0, idx));
    if (colorSpace != ColorSpace::RGB) {
        std::tuple<float*, float, float> editChannel = {nullptr, 0.0f, getMaxChannelValue(colorSpace, channel)};
        wrapAround(&mod.difference, editChannel);
    }
    if (idx == numberStr.size()) return mod;

    if (!((colorSpace == ColorSpace::HSL || colorSpace == ColorSpace::OKLCH) && channel == 'h')) {
        std::cout << "Ignoring units: " << arg << "\n";
        return mod;
    }

    std::string units = numberStr.substr(idx);
    std::ranges::transform(units, units.begin(),
                           [](const unsigned char c){ return std::tolower(c); });
    if (units == "rad") mod.difference *= 180 / M_PI;
    else if (units == "pirad") mod.difference *= 180;
    else if (units == "deg") {}
    else std::cout << "Unknown units\n";

    return mod;
}

inline std::array<Modifier, 3> parseColorSpace(Options& options, bool& err) {
    const std::array<std::function<void(uint8_t*, uint8_t*, uint8_t*, std::array<Modifier, 3>&)>, 4> colorSpaceValues = {
        modifyRGB,
        modifyHSL,
        modifyOKLAB,
        modifyOKLCh
    };

    std::string channelsSeen;

    options.modifierFunc = colorSpaceValues.at(getColorSpaceIndex(options.colorSpace));
    const std::string& arg = options.modifierArg;

    constexpr char delim = ';';
    std::stringstream ss(arg);
    std::string token;

    bool error = false;
    std::array<Modifier, 3> modifiers{};
    uint8_t i = 0;
    while (std::getline(ss, token, delim) && i++ < modifiers.max_size()) {
        const bool upper = token.at(0) > 64 && token.at(0) < 97;
        error = token.length() < 3 ||
            token.at(1) != ':' ||
            (!VALID_CHANNELS_MAP.at(getColorSpaceIndex(options.colorSpace)).contains(token.at(0)) && !channelsSeen.contains(token.at(0)));

        if (error) {
            std::cerr << "Error parsing color space\n";
            return {};
        }

        channelsSeen.append(std::format("{}{}", token.at(0), upper ? (char)(token.at(0) + 32) : (char)(token.at(0) - 32)));

        const std::string& mod = token.substr(2);
        modifiers[i - 1] = parseModifierArg(mod, options.colorSpace, token.at(0));

        if (modifiers.at(i-1).operation == Operation::INVALID) {
            err = true;
            break;
        }
    }

    return modifiers;
}
