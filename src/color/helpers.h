#pragma once
#include <algorithm>
#include <cmath>
#include <tuple>
#include <cstdint>
#include <stdexcept>

constexpr float OKLAB_EXPONENT = 2.4f;
constexpr float OKLAB_EXPONENT_INVERSE = 1.0f/OKLAB_EXPONENT;

inline std::tuple<float, float, float> getHSLFromRGB(uint8_t r, uint8_t g, uint8_t b) {
    double rf = (double)r / 255.0f, gf = (double)g / 255.0f, bf = (double)b / 255.0f;
    double max = std::max({rf, gf, bf});
    double min = std::min({rf, gf, bf});
    double delta = max - min;

    double h = 0.0f;
    if (delta != 0.0f) {
        if (max == rf) h = 60.0f * std::fmod(((gf - bf) / delta), 6.0f);
        else if (max == gf) h = 60.0f * (((bf - rf) / delta) + 2.0f);
        else h = 60.0f * (((rf - gf) / delta) + 4.0f);
    }
    if (h < 0.0f) h += 360.0f;

    double l = (max + min) / 2.0f;
    double s = (delta == 0.0f) ? 0.0f : delta / (1.0f - std::abs(2.0f * l - 1.0f));

    return {h, s*100, l*100};
}


inline std::tuple<uint8_t, uint8_t, uint8_t> getRGBFromHSL(float h, float s, float l) {
    s /= 100;
    l /= 100;

    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;

    double c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    double x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    double m = l - c / 2.0f;

    double r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else { r = c; b = x; }

    auto toByte = [](double val) -> uint8_t {
        return static_cast<uint8_t>(std::round(std::clamp(val * 255.0, 0.0, 255.0)));
    };

    return {toByte(r + m), toByte(g + m), toByte(b + m)};
}


inline std::tuple<float, float, float> rgbToOklab(uint8_t r, uint8_t g, uint8_t b) {
    auto toLinear = [](uint8_t channel) -> double {
        double normalized = static_cast<double>(channel) / 255.0f;
        return (normalized <= 0.04045f) ? normalized / 12.92f : std::pow((normalized + 0.055f) / 1.055f, OKLAB_EXPONENT);
    };

    double rLinear = toLinear(r);
    double gLinear = toLinear(g);
    double bLinear = toLinear(b);

    double l = 0.4122214708f * rLinear + 0.5363325363f * gLinear + 0.0514459929f * bLinear;
    double m = 0.2119034982f * rLinear + 0.6806995451f * gLinear + 0.1073969566f * bLinear;
    double s = 0.0883024619f * rLinear + 0.2817188376f * gLinear + 0.6299787005f * bLinear;

    l = std::cbrt(l);
    m = std::cbrt(m);
    s = std::cbrt(s);

    double L = 0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s;
    double A = 1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s;
    double B = 0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s;

    return {L, A, B};
}

inline std::tuple<uint8_t, uint8_t, uint8_t> OklabToRGB(float L, float A, float B) {
    double l = L + 0.3963377774f * A + 0.2158037573f * B;
    double m = L - 0.1055613458f * A - 0.0638541728f * B;
    double s = L - 0.0894841775f * A - 1.2914855480f * B;

    l = l * l * l;
    m = m * m * m;
    s = s * s * s;

    double rLinear =  4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    double gLinear = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    double bLinear = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

    auto toSRGB = [](double channel) {
        channel = std::clamp(channel, 0.0, 1.0);
        return (channel <= 0.0031308f) ? 12.92f * channel : 1.055f * std::pow(channel, OKLAB_EXPONENT_INVERSE) - 0.055f;
    };

    uint8_t r = static_cast<uint8_t>(std::round(std::clamp(toSRGB(rLinear) * 255.0, 0.0, 255.0)));
    uint8_t g = static_cast<uint8_t>(std::round(std::clamp(toSRGB(gLinear) * 255.0, 0.0, 255.0)));
    uint8_t b = static_cast<uint8_t>(std::round(std::clamp(toSRGB(bLinear) * 255.0, 0.0, 255.0)));

    return {r, g, b};
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