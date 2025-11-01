#pragma once
#include "datatypes/ColorSpace.h"

#include <array>
#include <stdexcept>

constexpr float PI = 3.14159265358979323846f;
constexpr std::array<std::array<float, 3>, 4> MAX_CHANNEL_VALS = {{
    {255.0f, 255.0f, 255.0f},
    {360.0f, 100.0f, 100.0f},
    {1.0f, 0.5f, 0.5f},
    {1.0f, 0.4f, 360.0f}
}};

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

constexpr int getColorSpaceIndex(const ColorSpace colorSpace) {
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

constexpr float getMaxChannelValue(const ColorSpace colorSpace, const char channel) {
  switch (colorSpace) {
  case ColorSpace::RGB:
    return MAX_CHANNEL_VALS[getColorSpaceIndex(colorSpace)][getRGBChannelIndex(channel)];
  case ColorSpace::HSL:
    return MAX_CHANNEL_VALS[getColorSpaceIndex(colorSpace)][getHSLChannelIndex(channel)];
  case ColorSpace::OKLAB:
    return MAX_CHANNEL_VALS[getColorSpaceIndex(colorSpace)][getOKLABChannelIndex(channel)];
  case ColorSpace::OKLCH:
    return MAX_CHANNEL_VALS[getColorSpaceIndex(colorSpace)][getOKLChChannelIndex(channel)];
  default: throw std::out_of_range("Invalid color space");
  }
}

template <typename T>
void wrapAround(T* value, std::tuple<T*, T, T>& channel) {
  while (*value < 0) *value += std::get<2>(channel);
  while (*value > std::get<2>(channel)) *value -= std::get<2>(channel);
}