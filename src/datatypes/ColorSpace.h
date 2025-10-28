#pragma once
#include <stdexcept>

enum class ColorSpace {
    UNSET = 0,
    RGB = 'r',
    HSL = 'u',
    OKLAB = 'l',
    OKLCH = 'c'
};

constexpr int getColorSpaceIndex(ColorSpace colorSpace) {
    switch (colorSpace) {
        case ColorSpace::RGB:
            return 0;
        case ColorSpace::HSL:
            return 1;
        case ColorSpace::OKLAB:
            return 2;
        case ColorSpace::OKLCH:
            return 3;
        default: throw std::out_of_range("Invalid color space");
    }
}
