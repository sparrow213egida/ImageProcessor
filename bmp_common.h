#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

constexpr uint8_t SizeOfBMPHeaderInBytes = 14;
constexpr uint32_t SizeOfDIBHeaderInBytes = 40;
constexpr uint8_t MaxChar = 255;
constexpr uint8_t BitsInBytes = 8;

inline double NumNormalize(double x) {
    return std::min(1.0, std::max(0.0, x));
}

inline double FromUnsignedCharToDouble(unsigned char x) {
    return static_cast<double>(x) / static_cast<double>(MaxChar);
}

inline unsigned char FromDoubleToUnsignedChar(double x) {
    return static_cast<unsigned char>(std::round(NumNormalize(x) * static_cast<double>(MaxChar)));
}