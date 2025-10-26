#include "colorModifiers.h"

#include <array>
#include <cmath>

#include "helpers.h"
#include "../datatypes/Quadruple.h"
#include "../datatypes/Triple.h"

void modifyRGB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, const bool overflowAllowed) {
    std::array<std::pair<uint8_t*, uint8_t>, 3> channels = {{
        {r, *r},
        {g, *g},
        {b, *b},
    }};
    std::pair<uint8_t*, uint8_t> invalid = {nullptr, 0};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        std::pair<uint8_t*, uint8_t>& editChannel = channels.at(getRGBChannelIndex(mod.editChannel));
        std::pair<uint8_t*, uint8_t>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getRGBChannelIndex(std::abs(mod.editChannel))) : invalid;
        if ((modifierNegative = mod.editChannel < 0)) modifierChannel.second = -modifierChannel.second;

        uint8_t* currentChannel = editChannel.first;
        switch (mod.operation) {
            case Operation::ADD:
                if (overflowAllowed) {
                    if (modifierChannel != invalid) *currentChannel += modifierChannel.second;
                    else *currentChannel += mod.difference;
                } else {
                    if (modifierChannel != invalid) *currentChannel += (uint8_t)std::clamp((float)editChannel.second, 0.0f,(float)(255 - modifierChannel.second));
                    else *currentChannel += (uint8_t)std::clamp(mod.difference, 0.0f,(float)(255 - editChannel.second));
                }
                break;
            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = ~modifierChannel.second;
                else *currentChannel = ~(uint8_t)mod.difference;
                break;
            case Operation::SET:
                if (modifierChannel != invalid) *currentChannel = editChannel.second;
                else if (overflowAllowed) *currentChannel = mod.difference;
                else *currentChannel = (uint8_t)std::clamp(mod.difference, 0.0f, 255.0f);

                break;
            case Operation::INVALID:
            default: ;
        }

        if (modifierNegative) modifierChannel.second = -modifierChannel.second;
    }
}

void modifyHSL(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, const bool overflowAllowed) {
    auto [h, s, l] = getHSLFromRGB(*r, *g, *b);

    std::array<Triple<float*, float, float>, 3> channels = {{
        {&h, h, 360.0f},
        {&s, s, 100.0f},
        {&l, l, 100.0f}
    }};
    Triple<float*, float, float> invalid = {nullptr, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        Triple<float*, float, float>& editChannel = channels.at(getHSLChannelIndex(mod.editChannel));
        Triple<float*, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getHSLChannelIndex(mod.modifierChannel)) : invalid;
        float* currentChannel = editChannel.first;

        if ((modifierNegative = mod.modifierChannel < 0)) modifierChannel.second = -modifierChannel.second;

        switch (mod.operation) {
            case Operation::ADD:
                if (overflowAllowed) *currentChannel += modifierChannel.second;
                else *currentChannel = std::clamp(*currentChannel + modifierChannel.second, 0.0f, modifierChannel.third);

                break;

            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = editChannel.third - modifierChannel.second;
                else *currentChannel = editChannel.third - mod.difference;

                break;

            case Operation::SET:
                if (mod.modifierChannel != 0) *currentChannel = modifierChannel.second;
                else if (overflowAllowed) *currentChannel = mod.difference;
                else *currentChannel = std::clamp(mod.difference, 0.0f, editChannel.third);

                break;

            case Operation::INVALID:
            default:
                break;
        }

        if (mod.editChannel == 'h') {
            while (*currentChannel < 0.0f) *currentChannel += 360.0f;
            while (*currentChannel >= 360.0f) *currentChannel -= 360.0f;
        }
        if (modifierNegative) modifierChannel.second = -modifierChannel.second;
    }

    std::tie(*r, *g, *b) = getRGBFromHSL(h, s, l);
}

void modifyOKLAB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, const bool overflowAllowed) {
    auto [l, a, b_] = rgbToOklab(*r, *g, *b);

    std::array<Quadruple<float*, float, float, float>, 3> channels = {{
        {&l, l, 0, 1},
        {&a, a, -0.25f, 0.25f},
        {&b_, b_, -0.25f, 0.25f}
    }};
    Quadruple<float*, float, float, float> invalid = {nullptr, 0.0f, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        Quadruple<float*, float, float, float>& editChannel = channels.at(getOKLABChannelIndex(mod.editChannel));
        Quadruple<float*, float, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getOKLABChannelIndex(std::abs(mod.modifierChannel))) : invalid;
        float* currentChannel = editChannel.first;

        if ((modifierNegative = mod.modifierChannel < 0)) modifierChannel.second = -modifierChannel.second;

        switch (mod.operation) {
            case Operation::ADD:
                if (overflowAllowed) *currentChannel += modifierChannel.second;
                else *currentChannel = std::clamp(*currentChannel + modifierChannel.second, editChannel.third, editChannel.fourth);

                break;

            case Operation::INVERT:
                if (mod.modifierChannel != 0) *currentChannel = editChannel.fourth - modifierChannel.second;
                else *currentChannel = editChannel.fourth - mod.difference;

                break;

            case Operation::SET:
                if (mod.modifierChannel != 0) *currentChannel = modifierChannel.second;
                else if (overflowAllowed) *currentChannel = mod.difference;
                else *currentChannel = std::clamp(mod.difference, editChannel.third, editChannel.fourth);

                break;

            case Operation::INVALID:
            default:
                break;
        }
        if (modifierNegative) modifierChannel.second = -modifierChannel.second;
    }

    std::tie(*r, *g, *b) = OklabToRGB(l, a, b_);
}
