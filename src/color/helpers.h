#pragma once
#include <algorithm>
#include <cmath>
#include <tuple>
#include <cstdint>
#include <stdexcept>

constexpr float OKLAB_EXPONENT = 2.4f;
constexpr float OKLAB_EXPONENT_INVERSE = 1.0f/OKLAB_EXPONENT;

inline std::tuple<float, float, float> RGBToHSL(uint8_t r, uint8_t g, uint8_t b) {
    float rf = (float)r / 255.0f, gf = (float)g / 255.0f, bf = (float)b / 255.0f;
    float max = std::max({rf, gf, bf});
    float min = std::min({rf, gf, bf});
    float delta = max - min;

    float h = 0.0f;
    if (delta != 0.0f) {
        if (max == rf) h = 60.0f * std::fmod(((gf - bf) / delta), 6.0f);
        else if (max == gf) h = 60.0f * (((bf - rf) / delta) + 2.0f);
        else h = 60.0f * (((rf - gf) / delta) + 4.0f);
    }
    if (h < 0.0f) h += 360.0f;

    float l = (max + min) / 2.0f;
    float s = (delta == 0.0f) ? 0.0f : delta / (1.0f - std::abs(2.0f * l - 1.0f));

    return {h, s*100, l*100};
}


inline std::tuple<uint8_t, uint8_t, uint8_t> HSLToRGB(float h, float s, float l) {
    s /= 100;
    l /= 100;

    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;

    float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else { r = c; b = x; }

    auto toByte = [](float val) -> uint8_t {
        return static_cast<uint8_t>(std::round(std::clamp(val * 255.0, 0.0, 255.0)));
    };

    return {toByte(r + m), toByte(g + m), toByte(b + m)};
}


inline std::tuple<float, float, float> RGBToOKLAB(uint8_t r, uint8_t g, uint8_t b) {
    auto toLinear = [](uint8_t channel) -> float {
        float normalized = static_cast<float>(channel) / 255.0f;
        return (normalized <= 0.04045f) ? normalized / 12.92f : std::pow((normalized + 0.055f) / 1.055f, OKLAB_EXPONENT);
    };

    float rLinear = toLinear(r);
    float gLinear = toLinear(g);
    float bLinear = toLinear(b);

    float l = 0.4122214708f * rLinear + 0.5363325363f * gLinear + 0.0514459929f * bLinear;
    float m = 0.2119034982f * rLinear + 0.6806995451f * gLinear + 0.1073969566f * bLinear;
    float s = 0.0883024619f * rLinear + 0.2817188376f * gLinear + 0.6299787005f * bLinear;

    l = std::cbrt(l);
    m = std::cbrt(m);
    s = std::cbrt(s);

    float L = 0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s;
    float A = 1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s;
    float B = 0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s;

    return {L, A, B};
}

inline std::tuple<uint8_t, uint8_t, uint8_t> OKLABToRGB(float L, float A, float B) {
    float l = L + 0.3963377774f * A + 0.2158037573f * B;
    float m = L - 0.1055613458f * A - 0.0638541728f * B;
    float s = L - 0.0894841775f * A - 1.2914855480f * B;

    l = l * l * l;
    m = m * m * m;
    s = s * s * s;

    float rLinear =  4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    float gLinear = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    float bLinear = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

    auto toSRGB = [](float channel) {
        channel = std::clamp(channel, 0.0f, 1.0f);
        return (channel <= 0.0031308f) ? 12.92f * channel : 1.055f * std::pow(channel, OKLAB_EXPONENT_INVERSE) - 0.055f;
    };

    uint8_t r = static_cast<uint8_t>(std::round(std::clamp(toSRGB(rLinear) * 255.0, 0.0, 255.0)));
    uint8_t g = static_cast<uint8_t>(std::round(std::clamp(toSRGB(gLinear) * 255.0, 0.0, 255.0)));
    uint8_t b = static_cast<uint8_t>(std::round(std::clamp(toSRGB(bLinear) * 255.0, 0.0, 255.0)));

    return {r, g, b};
}

inline std::tuple<float, float, float> RGBToOKLCh(uint8_t r, uint8_t g, uint8_t b) {
    auto [L, A, B] = RGBToOKLAB(r, g, b);

    float C = std::sqrt(A*A + B*B);
    // H is in degrees
    float h = std::atan2(B, A) * (180.0f / M_PI);

    return {L, C, h};
}

inline std::tuple<uint8_t, uint8_t, uint8_t> OKLChToRGB(float L, float C, float h) {
    h *= M_PI / 180.0f;

    float A = C * std::cos(h);
    float B = C * std::sin(h);

    return OKLABToRGB(L, A, B);
}

constexpr int getRGBChannelIndex(const char ch) {
    switch (ch) {
        case 'r': return 0;
        case 'g': return 1;
        case 'b': return 2;
        default: throw std::out_of_range("Invalid channel character");
    }
}

constexpr int getHSLChannelIndex(const char ch) {
    switch (ch) {
        case 'h': return 0;
        case 's': return 1;
        case 'l': return 2;
        default: throw std::out_of_range("Invalid channel character");
    }
}

constexpr int getOKLABChannelIndex(const char ch) {
    switch (ch) {
        case 'l': return 0;
        case 'a': return 1;
        case 'b': return 2;
        default: throw std::out_of_range("Invalid channel character");
    }
}

constexpr int getOKLChChannelIndex(const char ch) {
    switch (ch) {
        case 'l': return 0;
        case 'c': return 1;
        case 'h': return 2;
        default: throw std::out_of_range("Invalid channel character");
    }
}