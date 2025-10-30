#include "colorModifiers.h"

#include <array>
#include <cmath>

#include "helpers.h"

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
    auto [h, s, l] = RGBToHSL(*r, *g, *b);

    std::array<std::tuple<float*, float, float>, 3> channels = {{
        {&h, h, 360.0f},
        {&s, s, 100.0f},
        {&l, l, 100.0f}
    }};
    std::tuple<float*, float, float> invalid = {nullptr, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        std::tuple<float*, float, float>& editChannel = channels.at(getHSLChannelIndex(mod.editChannel));
        std::tuple<float*, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getHSLChannelIndex(mod.modifierChannel)) : invalid;
        float* currentChannel = std::get<0>(editChannel);

        if ((modifierNegative = mod.modifierChannel < 0)) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);

        switch (mod.operation) {
            case Operation::ADD:
                if (overflowAllowed) *currentChannel += std::get<1>(modifierChannel);
                else *currentChannel = std::clamp(*currentChannel + std::get<1>(modifierChannel), 0.0f, std::get<2>(modifierChannel));

                break;

            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = std::get<2>(editChannel) - std::get<1>(modifierChannel);
                else *currentChannel = std::get<2>(editChannel) - mod.difference;

                break;

            case Operation::SET:
                if (mod.modifierChannel != 0) *currentChannel = std::get<1>(modifierChannel);
                else if (overflowAllowed) *currentChannel = mod.difference;
                else *currentChannel = std::clamp(mod.difference, 0.0f, std::get<2>(editChannel));

                break;

            case Operation::INVALID:
            default:
                break;
        }

        if (mod.editChannel == 'h') {
            while (*currentChannel < 0.0f) *currentChannel += 360.0f;
            while (*currentChannel >= 360.0f) *currentChannel -= 360.0f;
        }
        if (modifierNegative) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);
    }

    std::tie(*r, *g, *b) = HSLToRGB(h, s, l);
}

void modifyOKLAB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, const bool overflowAllowed) {
    auto [l, a, b_] = RGBToOKLAB(*r, *g, *b);

    std::array<std::tuple<float*, float, float, float>, 3> channels = {{
        {&l, l, 0, 1},
        {&a, a, -0.25f, 0.25f},
        {&b_, b_, -0.25f, 0.25f}
    }};
    std::tuple<float*, float, float, float> invalid = {nullptr, 0.0f, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        std::tuple<float*, float, float, float>& editChannel = channels.at(getOKLABChannelIndex(mod.editChannel));
        std::tuple<float*, float, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getOKLABChannelIndex(std::abs(mod.modifierChannel))) : invalid;
        float* currentChannel = std::get<0>(editChannel);

        if ((modifierNegative = mod.modifierChannel < 0)) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);

        switch (mod.operation) {
            case Operation::ADD:
                if (overflowAllowed) *currentChannel += std::get<1>(modifierChannel);
                else *currentChannel = std::clamp(*currentChannel + std::get<1>(modifierChannel), std::get<2>(editChannel), std::get<3>(editChannel));

                break;

            case Operation::INVERT:
                if (mod.modifierChannel != 0) *currentChannel = std::get<3>(editChannel) - std::get<1>(modifierChannel);
                else *currentChannel = std::get<3>(editChannel) - mod.difference;

                break;

            case Operation::SET:
                if (mod.modifierChannel != 0) *currentChannel = std::get<1>(modifierChannel);
                else if (overflowAllowed) *currentChannel = mod.difference;
                else *currentChannel = std::clamp(mod.difference, std::get<2>(editChannel), std::get<3>(editChannel));

                break;

            case Operation::INVALID:
            default:
                break;
        }
        if (modifierNegative) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);
    }

    std::tie(*r, *g, *b) = OKLABToRGB(l, a, b_);
}

void modifyOKLCh(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, const bool overflowAllowed) {
    auto [l, c, h] = RGBToOKLCh(*r, *g, *b);

    std::array<std::tuple<float*, float, float, float>, 3> channels = {{
        {&l, l, 0, 1},
        {&c, c, 0.0f, 0.4f},
        {&h, h, 0.f, 360.0f}
    }};
    std::tuple<float*, float, float, float> invalid = {nullptr, 0.0f, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        std::tuple<float*, float, float, float>& editChannel = channels.at(getOKLChChannelIndex(mod.editChannel));
        std::tuple<float*, float, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getOKLChChannelIndex(std::abs(mod.modifierChannel))) : invalid;
        float* currentChannel = std::get<0>(editChannel);

        if ((modifierNegative = mod.modifierChannel < 0)) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);

        switch (mod.operation) {
            case Operation::ADD:
                if (overflowAllowed) *currentChannel += std::get<1>(modifierChannel);
                else *currentChannel = std::clamp(*currentChannel + std::get<1>(modifierChannel), std::get<2>(editChannel), std::get<3>(editChannel));

                break;

            case Operation::INVERT:
                if (mod.modifierChannel != 0) *currentChannel = std::get<3>(editChannel) - std::get<1>(modifierChannel);
                else *currentChannel = std::get<3>(editChannel) - mod.difference;

                break;

            case Operation::SET:
                if (mod.modifierChannel != 0) *currentChannel = std::get<1>(modifierChannel);
                else if (overflowAllowed) *currentChannel = mod.difference;
                else *currentChannel = std::clamp(mod.difference, std::get<2>(editChannel), std::get<3>(editChannel));

                break;

            case Operation::INVALID:
            default:
                break;
        }
        if (modifierNegative) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);
    }

    std::tie(*r, *g, *b) = OKLChToRGB(l, c, h);
}
