#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <tuple>

void rgbInvert(uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = ~*r;
    *g = ~*g;
    *b = ~*b;
}

float fpow(float x, float y) {
    int32_t i = *(int32_t*)&x;
    i = (int32_t)(y * (i - 1064866805) + 1064866805);
    return *(float*)&i;
}

float fatan2(float y, float x) {
    constexpr float QTR_PI = M_PI;
    constexpr float QTR_PI_3 = M_PI/3;
    float abs_y = fabsf(y) + 1e-10f;
    float angle;

    if (x >= 0.0f) {
        float r = (x - abs_y) / (x + abs_y);
        angle = QTR_PI - QTR_PI * r;
    } else {
        float r = (x + abs_y) / (abs_y - x);
        angle = QTR_PI_3 - QTR_PI * r;
    }

    return (y < 0.0f) ? -angle : angle;
}


std::tuple<float, float, float> getHSLFromRGB(uint8_t r, uint8_t g, uint8_t b) {
    float xR = (float)r / 255.0f;
    float xG = (float)g / 255.0f;
    float xB = (float)b / 255.0f;

    float max = (xR > xB) ? ((xR > xB) ? xR : xB) : ((xG > xB) ? xG : xB);
    float min = (xR < xB) ? ((xR < xB) ? xR : xB) : ((xG < xB) ? xG : xB);
            
    float hue;
    if (max == min) hue = 0;
    else if(max == xR) hue = std::fmod(60 * ((xG - xB) / (max - min)), 360);
    else if(max == xG) hue = 60 * ((xB - xR) / (max - min)) + 120;
    else hue = 60 * ((xR - xG) / (max - min)) + 240;

    float light = (max + min) / 2.0f;
            
    float saturation;
    if(light < 0.5f) saturation = (max - min) / (max + min);
    else saturation = (max - min) / (2.0f - max - min);

    return {hue, saturation, light};
}

std::tuple<uint8_t, uint8_t, uint8_t> getRGBFromHSL(float h, float s, float l) {
    float chroma = (1 - std::abs(2*l - 1)) * s;
    float intermediate = chroma * (1 - std::abs(std::fmod((h / 60), 2) - 1));

    float xR = 0, xG = 0, xB = 0;
    if(h < 60){
        xR = chroma;
        xG = intermediate;
    } else if(h < 120) {
        xR = intermediate;
        xG = chroma;
    } else if(h < 180) {
        xG = chroma;
        xB = intermediate;
    } else if(h < 240) {
        xG = intermediate;
        xB = chroma;
    } else if(h < 300) {
        xR = intermediate;
        xB = chroma;
    } else {
        xR = chroma;
        xB = intermediate;
    }

    float lightnessOffset = l - chroma / 2.0f;
    return {(uint8_t)((xR + lightnessOffset) * 255), (uint8_t)((xG + lightnessOffset) * 255), (uint8_t)((xB + lightnessOffset) * 255)};
}

void hueInvert(uint8_t* r, uint8_t* g, uint8_t* b) {
    auto [hue, light, saturation] = getHSLFromRGB(*r, *g, *b);
    hue = (uint16_t)std::fmod(hue + 180.0f, 360.0f);

    auto [rNew, gNew, bNew] = getRGBFromHSL(hue, light, saturation);
    *r = rNew;
    *g = gNew;
    *b = bNew;
}


std::tuple<float, float, float> rgbToOklab(uint8_t r, uint8_t g, uint8_t b) {
    auto toLinear = [](uint8_t channel) {
        float normalized = (float)channel / 255.0f;
        return (normalized <= 0.0405f) ? normalized / 12.92f : fpow((normalized + 0.055f) / 1.055f, 2.4f);
    };

    float rLinear = toLinear(r);
    float gLinear = toLinear(g);
    float bLinear = toLinear(b);

    constexpr float LMSMatrix[3][3] = {
        {0.4122214708f, 0.5363325363f, 0.0514459929f},
        {0.2119034982f, 0.6806995451f, 0.1073969566f},
        {0.0883024619f, 0.2817188376f, 0.6299787005f}
    };

    float l = LMSMatrix[0][0]*rLinear + LMSMatrix[0][1]*gLinear + LMSMatrix[0][2]*bLinear;
    float m = LMSMatrix[1][0]*rLinear + LMSMatrix[1][1]*gLinear + LMSMatrix[1][2]*bLinear;
    float s = LMSMatrix[2][0]*rLinear + LMSMatrix[2][1]*gLinear + LMSMatrix[2][2]*bLinear;

    l = std::cbrt(l);
    m = std::cbrt(m);
    s = std::cbrt(s);

    constexpr float OklabMatrix[3][3] = {
        {0.2104542553f, 0.7936177850f, -0.0040720468f},
        {1.9779984951f, -2.4285922050f, 0.4505937099f},
        {0.0259040371f, 0.7827717662f, -0.8086757660f}
    };

    return {OklabMatrix[0][0]*l + OklabMatrix[0][1]*m + OklabMatrix[0][2]*s, OklabMatrix[1][0]*l + OklabMatrix[1][1]*m + OklabMatrix[1][2]*s, OklabMatrix[2][0]*l + OklabMatrix[2][1]*m + OklabMatrix[2][2]*s};
}

std::tuple<uint8_t, uint8_t, uint8_t> OklabToRGB(float L, float A, float B) {
    float l = L + 0.3963377774f * A + 0.2158037573f * B;
    float m = L - 0.1055613458f * A - 0.0638541728f * B;
    float s = L - 0.0894841775f * A - 1.2914855480f * B;

    l = l*l*l;
    m = m*m*m;
    s = s*s*s;

    constexpr float RGBInv[3][3] = {
        {4.0767416621f, -3.3077115913f, 0.2309699292f},
        {-1.2684380046f, 2.6097574011f, -0.3413193965f},
        {-0.0041960863f, -0.7034186147f, 1.7076147010f}
    };

    float rLinear = RGBInv[0][0]*l + RGBInv[0][1]*m + RGBInv[0][2]*s;
    float gLinear = RGBInv[1][0]*l + RGBInv[1][1]*m + RGBInv[1][2]*s;
    float bLinear = RGBInv[2][0]*l + RGBInv[2][1]*m + RGBInv[2][2]*s;

    auto toSRGB = [](float channel) {
        if (channel <= 0.0031308f) return 12.92f * channel;
        return 1.055f * fpow(channel, 1.0f / 2.4f) - 0.055f;
    };

    return {(uint8_t)std::clamp(toSRGB(rLinear) * 255, 0.0f, 255.0f), (uint8_t)std::clamp(toSRGB(gLinear) * 255, 0.0f, 255.0f), (uint8_t)std::clamp(toSRGB(bLinear) * 255, 0.0f, 255.0f)};
}

void oklabInvert(uint8_t* r, uint8_t* g, uint8_t* b) {
    auto [L, A, B] = rgbToOklab(*r, *g, *b);

    float chroma = std::sqrt(A*A + B*B);
    float hue = fatan2(B, A);

    if (hue <= M_PI) hue -= M_PI;
    else hue += M_PI;

    A = chroma * std::cos(hue);
    B = chroma * std::sin(hue);

    std::tie(*r, *g, *b) = OklabToRGB(L, A, B);
}

void oklabFlip(uint8_t* r, uint8_t* g, uint8_t* b) {
    auto [L, A, B] = rgbToOklab(*r, *g, *b);

    auto* pA = (uint32_t*)&A;
    auto* pB = (uint32_t*)&B;

    *pA = *pA ^ *pB;
    *pB = *pA ^ *pB;
    *pA = *pA ^ *pB;

    std::tie(*r, *g, *b) = OklabToRGB(L, A, B);
}