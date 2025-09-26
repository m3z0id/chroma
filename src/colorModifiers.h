#include <cstdint>
#include <cmath>
#include <algorithm>
#include <tuple>

#include <print>

void rgbInvert(uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = ~*r;
    *g = ~*g;
    *b = ~*b;
}

std::tuple<double, double, double> getHSLFromRGB(uint8_t r, uint8_t g, uint8_t b) {
    double xR = (double)r / 255.0;
    double xG = (double)g / 255.0;
    double xB = (double)b / 255.0;

    double max = std::max({xR, xG, xB});
    double min = std::min({xR, xG, xB});
            
    double hue;
    if (max == min) hue = 0;
    else if(max == xR) hue = std::fmod(60 * ((xG - xB) / (max - min)), 360);
    else if(max == xG) hue = 60 * ((xB - xR) / (max - min)) + 120;
    else hue = 60 * ((xR - xG) / (max - min)) + 240;

    double light = (max + min) / 2;
            
    double saturation;
    if(light < 0.5) saturation = (max - min) / (max + min);
    else saturation = (max - min) / (2.0 - max - min);

    return {hue, saturation, light};
}

std::tuple<uint8_t, uint8_t, uint8_t> getRGBFromHSL(double h, double s, double l) {
    double chroma = (1 - std::abs(2*l - 1)) * s;
    double intermediate = chroma * (1 - std::abs(std::fmod((h / 60), 2) - 1));

    double xR = 0, xG = 0, xB = 0;
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

    double lightnessOffset = l - chroma / 2;
    return {(uint8_t)((xR + lightnessOffset) * 255), (uint8_t)((xG + lightnessOffset) * 255), (uint8_t)((xB + lightnessOffset) * 255)};
}

void hueInvert(uint8_t* r, uint8_t* g, uint8_t* b) {
    auto [hue, light, saturation] = getHSLFromRGB(*r, *g, *b);
    hue = (uint16_t)std::fmod(hue + 180.0, 360.0);

    auto [rNew, gNew, bNew] = getRGBFromHSL(hue, light, saturation);
    *r = rNew;
    *g = gNew;
    *b = bNew;
}


std::tuple<double, double, double> rgbToOklab(uint8_t r, uint8_t g, uint8_t b) {
    auto toLinear = [](uint8_t channel) {
        double normalized = channel / 255.0;
        return (normalized <= 0.0405) ? normalized / 12.92 : std::pow((normalized + 0.055) / 1.055, 2.4);
    };

    double rLinear = toLinear(r);
    double gLinear = toLinear(g);
    double bLinear = toLinear(b);

    constexpr double LMSMatrix[3][3] = {
        {0.4122214708, 0.5363325363, 0.0514459929},
        {0.2119034982, 0.6806995451, 0.1073969566},
        {0.0883024619, 0.2817188376, 0.6299787005}
    };

    double l = LMSMatrix[0][0]*rLinear + LMSMatrix[0][1]*gLinear + LMSMatrix[0][2]*bLinear;
    double m = LMSMatrix[1][0]*rLinear + LMSMatrix[1][1]*gLinear + LMSMatrix[1][2]*bLinear;
    double s = LMSMatrix[2][0]*rLinear + LMSMatrix[2][1]*gLinear + LMSMatrix[2][2]*bLinear;

    l = std::cbrt(l);
    m = std::cbrt(m);
    s = std::cbrt(s);

    constexpr double OklabMatrix[3][3] = {
        {0.2104542553, 0.7936177850, -0.0040720468},
        {1.9779984951, -2.4285922050, 0.4505937099},
        {0.0259040371, 0.7827717662, -0.8086757660}
    };

    return {OklabMatrix[0][0]*l + OklabMatrix[0][1]*m + OklabMatrix[0][2]*s, OklabMatrix[1][0]*l + OklabMatrix[1][1]*m + OklabMatrix[1][2]*s, OklabMatrix[2][0]*l + OklabMatrix[2][1]*m + OklabMatrix[2][2]*s};
}

std::tuple<uint8_t, uint8_t, uint8_t> OklabToRGB(double L, double A, double B) {
    double l = L + 0.3963377774 * A + 0.2158037573 * B;
    double m = L - 0.1055613458 * A - 0.0638541728 * B;
    double s = L - 0.0894841775 * A - 1.2914855480 * B;

    l = l*l*l;
    m = m*m*m;
    s = s*s*s;

    constexpr double RGBInv[3][3] = {
        {4.0767416621, -3.3077115913, 0.2309699292},
        {-1.2684380046, 2.6097574011, -0.3413193965},
        {-0.0041960863, -0.7034186147, 1.7076147010}
    };

    double rLinear = RGBInv[0][0]*l + RGBInv[0][1]*m + RGBInv[0][2]*s;
    double gLinear = RGBInv[1][0]*l + RGBInv[1][1]*m + RGBInv[1][2]*s;
    double bLinear = RGBInv[2][0]*l + RGBInv[2][1]*m + RGBInv[2][2]*s;

    auto toSRGB = [](double channel) {
        if (channel <= 0.0031308) return 12.92 * channel;
        return 1.055 * std::pow(channel, 1.0 / 2.4) - 0.055;
    };

    return {(uint8_t)std::clamp(toSRGB(rLinear) * 255, 0.0, 255.0), (uint8_t)std::clamp(toSRGB(gLinear) * 255, 0.0, 255.0), (uint8_t)std::clamp(toSRGB(bLinear) * 255, 0.0, 255.0)};
}

void oklabInvert(uint8_t* r, uint8_t* g, uint8_t* b) {
    auto [L, A, B] = rgbToOklab(*r, *g, *b);

    double chroma = std::sqrt(A*A + B*B);
    double hue = std::atan2(B, A);

    if (hue <= M_PI) hue -= M_PI;
    else hue += M_PI;

    A = chroma * std::cos(hue);
    B = chroma * std::sin(hue);

    std::tie(*r, *g, *b) = OklabToRGB(L, A, B);
}

void oklabFlip(uint8_t* r, uint8_t* g, uint8_t* b) {
    auto [L, A, B] = rgbToOklab(*r, *g, *b);

    uint64_t* pA = (uint64_t*)&A;
    uint64_t* pB = (uint64_t*)&B;

    *pA = *pA ^ *pB;
    *pB = *pA ^ *pB;
    *pA = *pA ^ *pB;

    std::tie(*r, *g, *b) = OklabToRGB(L, A, B);
}