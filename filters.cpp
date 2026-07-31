#include "filters.h"

#include <cmath>
#include <iostream>

namespace {

constexpr uint8_t DefaultBitsPerPixel = 24;
constexpr uint8_t NumberOfElementsInMatrix = 9;
constexpr double Accuracy = 1e-6;

PixelArray ApplyMatrixFilter(const char (&matrix)[NumberOfElementsInMatrix], const PixelArray& input) {
    const int32_t width = input.GetWidth();
    const int32_t height = input.GetHeight();
    const uint16_t bits_per_pixel = input.GetBPP();
    const size_t channels = static_cast<size_t>(bits_per_pixel) / BitsInBytes;
    const auto& src = input.GetMatrix();

    std::vector<std::vector<Pixel>> extended(static_cast<size_t>(height) + 2,
                                             std::vector<Pixel>(static_cast<size_t>(width) + 2, Pixel(bits_per_pixel)));

    for (int32_t i = 0; i < height; ++i) {
        for (int32_t j = 0; j < width; ++j) {
            extended[static_cast<size_t>(i) + 1][static_cast<size_t>(j) + 1] =
                src[static_cast<size_t>(i)][static_cast<size_t>(j)];
        }
    }

    for (int32_t i = 0; i < height + 2; ++i) {
        extended[static_cast<size_t>(i)][0] = extended[static_cast<size_t>(i)][1];
        extended[static_cast<size_t>(i)][static_cast<size_t>(width) + 1] =
            extended[static_cast<size_t>(i)][static_cast<size_t>(width)];
    }
    for (int32_t j = 0; j < width + 2; ++j) {
        extended[0][static_cast<size_t>(j)] = extended[1][static_cast<size_t>(j)];
        extended[static_cast<size_t>(height) + 1][static_cast<size_t>(j)] =
            extended[static_cast<size_t>(height)][static_cast<size_t>(j)];
    }

    auto result = src;
    for (int32_t i = 0; i < height; ++i) {
        for (int32_t j = 0; j < width; ++j) {
            std::vector<double> new_values(channels, 0.0);
            for (int32_t dy = 0; dy < 3; ++dy) {
                for (int32_t dx = 0; dx < 3; ++dx) {
                    const auto& values = extended[static_cast<size_t>(i) + static_cast<size_t>(dy)]
                                                 [static_cast<size_t>(j) + static_cast<size_t>(dx)]
                                                     .GetValues();
                    const double coeff = matrix[static_cast<size_t>(dy) * 3 + static_cast<size_t>(dx)];
                    for (size_t c = 0; c < channels; ++c) {
                        new_values[c] += values[c] * coeff;
                    }
                }
            }
            for (auto& value : new_values) {
                value = std::min(1.0, std::max(0.0, value));
            }
            result[static_cast<size_t>(i)][static_cast<size_t>(j)] = Pixel(bits_per_pixel, new_values);
        }
    }
    return PixelArray(width, height, bits_per_pixel, input.GetPadding(), result);
}

}  // namespace

PixelArray GrayScaleFilter::Apply(const PixelArray& input) {
    auto matrix = input.GetMatrix();
    for (auto& row : matrix) {
        for (auto& pixel : row) {
            auto& values = pixel.GetValues();
            const double gray = 0.299 * values[2] + 0.587 * values[1] + 0.114 * values[0];
            for (auto& value : values) {
                value = gray;
            }
        }
    }
    return PixelArray(input.GetWidth(), input.GetHeight(), input.GetBPP(), input.GetPadding(), matrix);
}

PixelArray NegativeFilter::Apply(const PixelArray& input) {
    auto matrix = input.GetMatrix();
    for (auto& row : matrix) {
        for (auto& pixel : row) {
            for (auto& value : pixel.GetValues()) {
                value = 1.0 - value;
            }
        }
    }
    return PixelArray(input.GetWidth(), input.GetHeight(), input.GetBPP(), input.GetPadding(), matrix);
}

PixelArray SharpeningFilter::Apply(const PixelArray& input) {
    const char matrix[NumberOfElementsInMatrix] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    return ApplyMatrixFilter(matrix, input);
}

PixelArray EdgeDetectionFilter::Apply(PixelArray input, double threshold) {
    input = GrayScaleFilter::Apply(input);
    const char matrix[NumberOfElementsInMatrix] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    input = ApplyMatrixFilter(matrix, input);

    auto matrix_data = input.GetMatrix();
    for (auto& row : matrix_data) {
        for (auto& pixel : row) {
            for (auto& value : pixel.GetValues()) {
                value = (value > threshold) ? 1.0 : 0.0;
            }
        }
    }
    return PixelArray(input.GetWidth(), input.GetHeight(), DefaultBitsPerPixel, input.GetPadding(), matrix_data);
}

PixelArray CropFilter::Apply(const PixelArray& input, int32_t new_width, int32_t new_height) {
    const int32_t width = input.GetWidth();
    const int32_t height = input.GetHeight();
    const uint16_t bits_per_pixel = input.GetBPP();
    const int32_t crop_width = std::min(new_width, width);
    const int32_t crop_height = std::min(new_height, height);
    const auto& src = input.GetMatrix();

    std::vector<std::vector<Pixel>> cropped(static_cast<size_t>(crop_height),
                                            std::vector<Pixel>(static_cast<size_t>(crop_width)));
    for (int32_t i = 0; i < crop_height; ++i) {
        for (int32_t j = 0; j < crop_width; ++j) {
            cropped[static_cast<size_t>(i)][static_cast<size_t>(j)] =
                src[static_cast<size_t>(height - crop_height + i)][static_cast<size_t>(j)];
        }
    }

    const uint8_t new_padding =
        static_cast<uint8_t>((bits_per_pixel / BitsInBytes * static_cast<uint32_t>(crop_width) + 3) / 4 * 4 -
                             bits_per_pixel / BitsInBytes * static_cast<uint32_t>(crop_width));
    return PixelArray(crop_width, crop_height, bits_per_pixel, new_padding, cropped);
}

PixelArray GaussianBlurFilter::Apply(const PixelArray& input, double sigma) {
    if (sigma < Accuracy) {
        return input;
    }

    const int32_t width = input.GetWidth();
    const int32_t height = input.GetHeight();
    const uint16_t bits_per_pixel = input.GetBPP();
    const size_t channels = static_cast<size_t>(bits_per_pixel) / BitsInBytes;
    const int32_t radius = static_cast<int32_t>(std::ceil(sigma * 3.0));
    const int32_t kernel_size = 2 * radius + 1;

    std::vector<double> kernel(static_cast<size_t>(kernel_size));
    double sum = 0.0;
    const double sigma_squared_2 = 2.0 * sigma * sigma;
    for (int32_t i = 0; i < kernel_size; ++i) {
        const int32_t x = i - radius;
        kernel[static_cast<size_t>(i)] = std::exp(-static_cast<double>(x * x) / sigma_squared_2);
        sum += kernel[static_cast<size_t>(i)];
    }
    for (auto& value : kernel) {
        value /= sum;
    }

    const auto& src = input.GetMatrix();
    auto temp = src;

    for (int32_t y = 0; y < height; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            std::vector<double> new_values(channels, 0.0);
            const int32_t left = std::max(0, x - radius);
            const int32_t right = std::min(width - 1, x + radius);
            for (int32_t kx = left; kx <= right; ++kx) {
                const auto& values = src[static_cast<size_t>(y)][static_cast<size_t>(kx)].GetValues();
                const double weight = kernel[static_cast<size_t>(std::abs(kx - x))];
                for (size_t c = 0; c < channels; ++c) {
                    new_values[c] += values[c] * weight;
                }
            }
            temp[static_cast<size_t>(y)][static_cast<size_t>(x)] = Pixel(bits_per_pixel, new_values);
        }
    }

    auto result = temp;
    for (int32_t x = 0; x < width; ++x) {
        for (int32_t y = 0; y < height; ++y) {
            std::vector<double> new_values(channels, 0.0);
            const int32_t top = std::max(0, y - radius);
            const int32_t bottom = std::min(height - 1, y + radius);
            for (int32_t ky = top; ky <= bottom; ++ky) {
                const auto& values = temp[static_cast<size_t>(ky)][static_cast<size_t>(x)].GetValues();
                const double weight = kernel[static_cast<size_t>(std::abs(ky - y))];
                for (size_t c = 0; c < channels; ++c) {
                    new_values[c] += values[c] * weight;
                }
            }
            result[static_cast<size_t>(y)][static_cast<size_t>(x)] = Pixel(bits_per_pixel, new_values);
        }
    }

    return PixelArray(width, height, bits_per_pixel, input.GetPadding(), result);
}

void PrintHelp() {
    std::cout << "Image Processor Usage:\n"
              << "  ./image_processor input.bmp output.bmp [filters...]\n\n"
              << "Available filters:\n"
              << "  -crop width height     Crop image\n"
              << "  -gs                    Convert to grayscale\n"
              << "  -neg                   Apply negative\n"
              << "  -sharp                 Sharpen image\n"
              << "  -edge threshold        Edge detection\n"
              << "  -blur sigma            Gaussian blur\n\n"
              << "Example:\n"
              << "  ./image_processor in.bmp out.bmp -crop 800 600 -gs -blur 0.5\n";
}