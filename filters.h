#pragma once

#include "bmp_structures.h"

class GrayScaleFilter {
public:
    static PixelArray Apply(const PixelArray &input);
};

class NegativeFilter {
public:
    static PixelArray Apply(const PixelArray &input);
};

class SharpeningFilter {
public:
    static PixelArray Apply(const PixelArray &input);
};

class EdgeDetectionFilter {
public:
    static PixelArray Apply(PixelArray input, double threshold);
};

class CropFilter {
public:
    static PixelArray Apply(const PixelArray &input, int32_t new_width, int32_t new_height);
};

class GaussianBlurFilter {
public:
    static PixelArray Apply(const PixelArray &input, double sigma);
};

void PrintHelp();