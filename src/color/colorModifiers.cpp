#include "colorModifiers.h"

#include <array>
#include <cmath>

#include "../Global.h"
#include "helpers.h"

void modifyRGB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods) {
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
                if (modifierChannel != invalid) *currentChannel += (uint8_t)std::clamp((float)editChannel.second, 0.0f,(float)(255 - modifierChannel.second));
                else *currentChannel += (uint8_t)std::clamp(mod.difference, 0.0f,(float)(255 - editChannel.second));
                break;
            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = ~modifierChannel.second;
                else *currentChannel = ~(uint8_t)mod.difference;
                break;
            case Operation::SET:
                if (modifierChannel != invalid) *currentChannel = editChannel.second;
                else *currentChannel = mod.difference;
                break;
            case Operation::INVALID:
            default: ;
        }

        if (modifierNegative) modifierChannel.second = -modifierChannel.second;
    }
}

void modifyHSL(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods) {
    auto [h, s, l] = RGBToHSL(*r, *g, *b);

    std::array<std::tuple<float*, float, float>, 3> channels = {{
        {&h, h, getMaxChannelValue(ColorSpace::HSL, 'h')},
        {&s, s, getMaxChannelValue(ColorSpace::HSL, 's')},
        {&l, l, getMaxChannelValue(ColorSpace::HSL, 'l')}
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
                if (modifierChannel != invalid) {
                    if (mod.editChannel == 'h') {
                        *currentChannel += std::get<1>(modifierChannel);
                        wrapAround(currentChannel, editChannel);
                    }
                    else *currentChannel = std::clamp(*currentChannel + std::get<1>(modifierChannel), 0.0f, std::get<2>(modifierChannel));
                } else *currentChannel += std::clamp(mod.difference, 0.0f,std::get<2>(editChannel));
                break;

            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = std::get<2>(editChannel) - std::get<1>(modifierChannel);
                else *currentChannel = std::get<2>(editChannel) - mod.difference;

                break;

            case Operation::SET:
                if (modifierChannel != invalid) *currentChannel = std::get<1>(modifierChannel);
                else *currentChannel = mod.difference;

                break;

            case Operation::INVALID:
            default:
                break;
        }
        if (modifierNegative) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);
    }

    std::tie(*r, *g, *b) = HSLToRGB(h, s, l);
}

void modifyOKLAB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods) {
    auto [l, a, b_] = RGBToOKLAB(*r, *g, *b);
    a += 0.25f;
    b_ += 0.25f;

    std::array<std::tuple<float*, float, float>, 3> channels = {{
        {&l, l, getMaxChannelValue(ColorSpace::OKLAB, 'l')},
        {&a, a, getMaxChannelValue(ColorSpace::OKLAB, 'a')},
        {&b_, b_, getMaxChannelValue(ColorSpace::OKLAB, 'b')}
    }};
    std::tuple<float*, float, float> invalid = {nullptr, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        std::tuple<float*, float, float>& editChannel = channels.at(getOKLABChannelIndex(mod.editChannel));
        std::tuple<float*, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getOKLABChannelIndex(std::abs(mod.modifierChannel))) : invalid;
        float* currentChannel = std::get<0>(editChannel);

        if ((modifierNegative = mod.modifierChannel < 0)) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);

        switch (mod.operation) {
            case Operation::ADD:
                if (modifierChannel != invalid) *currentChannel = std::clamp(*currentChannel + std::get<1>(modifierChannel), 0.0f, std::get<2>(modifierChannel));
                else *currentChannel += std::clamp(mod.difference, 0.0f,std::get<2>(editChannel));

                break;

            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = std::get<2>(editChannel) - std::get<1>(modifierChannel);
                else *currentChannel = std::get<2>(editChannel) - mod.difference;

                break;

            case Operation::SET:
                if (modifierChannel != invalid) *currentChannel = std::get<1>(modifierChannel);
                else *currentChannel = mod.difference;

                break;

            case Operation::INVALID:
            default:
                break;
        }
        if (modifierNegative) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);
    }

    std::tie(*r, *g, *b) = OKLABToRGB(l, a - 0.25f, b_ - 0.25f);
}

void modifyOKLCh(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods) {
    auto [l, c, h] = RGBToOKLCh(*r, *g, *b);
    std::array<std::tuple<float*, float, float>, 3> channels = {{
        {&l, l, getMaxChannelValue(ColorSpace::OKLCH, 'l')},
        {&c, c, getMaxChannelValue(ColorSpace::OKLCH, 'c')},
        {&h, h, getMaxChannelValue(ColorSpace::OKLCH, 'h')}
    }};
    std::tuple<float*, float, float> invalid = {nullptr, 0.0f, 0.0f};

    for (const auto& mod : mods) {
        if (mod.operation == Operation::INVALID) continue;

        bool modifierNegative = false;
        std::tuple<float*, float, float>& editChannel = channels.at(getOKLChChannelIndex(mod.editChannel));
        std::tuple<float*, float, float>& modifierChannel = mod.modifierChannel != 0 ? channels.at(getOKLChChannelIndex(std::abs(mod.modifierChannel))) : invalid;
        float* currentChannel = std::get<0>(editChannel);

        if ((modifierNegative = mod.modifierChannel < 0)) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);

        switch (mod.operation) {
            case Operation::ADD:
                if (modifierChannel != invalid) {
                    if (mod.editChannel == 'h') {
                        *currentChannel += std::get<1>(modifierChannel);
                        wrapAround(currentChannel, editChannel);
                    }
                    else *currentChannel = std::clamp(*currentChannel + std::get<1>(modifierChannel), 0.0f, std::get<2>(modifierChannel));
                } else *currentChannel += std::clamp(mod.difference, 0.0f,std::get<2>(editChannel));
                break;

            case Operation::INVERT:
                if (modifierChannel != invalid) *currentChannel = std::get<2>(editChannel) - std::get<1>(modifierChannel);
                else *currentChannel = std::get<2>(editChannel) - mod.difference;

                break;

            case Operation::SET:
                if (modifierChannel != invalid) *currentChannel = std::get<1>(modifierChannel);
                *currentChannel = mod.difference;

                break;

            case Operation::INVALID:
            default:
                break;
        }
        if (modifierNegative) std::get<1>(modifierChannel) = -std::get<1>(modifierChannel);
    }

    std::tie(*r, *g, *b) = OKLChToRGB(l, c, h);
}