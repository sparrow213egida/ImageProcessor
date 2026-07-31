#pragma once

#include "bmp_common.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class Pixel {
public:
    Pixel() = default;
    explicit Pixel(uint16_t bits_per_pixel);
    Pixel(uint16_t bits_per_pixel, const std::vector<double> &values);

    [[nodiscard]] uint16_t GetBPP() const;
    [[nodiscard]] const std::vector<double> &GetValues() const;
    std::vector<double> &GetValues();

private:
    uint16_t bits_per_pixel_ = 0;
    std::vector<double> values_;
};

class PixelArray {
public:
    PixelArray() = default;
    PixelArray(int32_t width, int32_t height, uint16_t bits_per_pixel, uint8_t padding,
               const std::vector<std::vector<Pixel>> &matrix);

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;
    [[nodiscard]] uint16_t GetBPP() const;
    [[nodiscard]] uint8_t GetPadding() const;
    [[nodiscard]] const std::vector<std::vector<Pixel>> &GetMatrix() const;
    std::vector<std::vector<Pixel>> &GetMatrix();

    void Save(std::ofstream &os) const;

private:
    int32_t width_ = 0;
    int32_t height_ = 0;
    uint16_t bits_per_pixel_ = 0;
    uint8_t padding_ = 0;
    std::vector<std::vector<Pixel>> matrix_;
};

class BMPHeader {
public:
    BMPHeader() = default;
    BMPHeader(uint32_t file_size, int32_t offset);

    void Save(std::ofstream &os) const;

private:
    char identifier_[2] = {'B', 'M'};
    uint32_t file_size_ = 0;
    int16_t first_reserved_ = 0;
    int16_t second_reserved_ = 0;
    int32_t offset_ = 0;
};

class DIBHeader {
public:
    DIBHeader() = default;
    DIBHeader(int32_t width, int32_t height, uint16_t bits_per_pixel, uint32_t image_size);

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;
    [[nodiscard]] uint16_t GetBPP() const;

    void Save(std::ofstream &os) const;

private:
    uint32_t header_size_ = SizeOfDIBHeaderInBytes;
    int32_t width_ = 0;
    int32_t height_ = 0;
    uint16_t planes_ = 1;
    uint16_t bits_per_pixel_ = 0;
    uint32_t compression_ = 0;
    uint32_t image_size_ = 0;
    int32_t horizontal_resolution_ = 0;
    int32_t vertical_resolution_ = 0;
    uint32_t colors_ = 0;
    uint32_t important_colors_ = 0;
};

class BmpFile {
public:
    BmpFile() = default;
    explicit BmpFile(const PixelArray &pixel_array);
    BmpFile(const BMPHeader &header, const DIBHeader &dib_header, const PixelArray &pixel_array);

    [[nodiscard]] PixelArray GetPixelArray() const;
    void Save(std::ofstream &os) const;

private:
    BMPHeader bmp_header_;
    DIBHeader dib_header_;
    PixelArray pixel_array_;
};

BMPHeader ReadBMPHeader(std::ifstream &input_stream);
DIBHeader ReadDIBHeader(std::ifstream &input_stream);
PixelArray ReadPixelArray(std::ifstream &input_stream, uint16_t bits_per_pixel, int32_t width, int32_t height);
BmpFile LoadBmp(const std::string &file_path);
void SaveBmp(const std::string &file_path, const BmpFile &bmp_file);