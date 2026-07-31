#include "bmp_structures.h"

#include <iostream>
#include <stdexcept>

template <typename T>
static void ReadImpl(std::ifstream &input_stream, T &value) {
    input_stream.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template <typename T>
static void WriteImpl(std::ofstream &output_stream, const T &value) {
    output_stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

static void WriteByte(std::ofstream &output_stream, unsigned char value) {
    output_stream.write(reinterpret_cast<const char*>(&value), 1);
}

Pixel::Pixel(uint16_t bits_per_pixel) : bits_per_pixel_(bits_per_pixel), values_(bits_per_pixel / BitsInBytes, 0.0) {
}

Pixel::Pixel(uint16_t bits_per_pixel, const std::vector<double> &values)
    : bits_per_pixel_(bits_per_pixel), values_(values) {
}

uint16_t Pixel::GetBPP() const {
    return bits_per_pixel_;
}

const std::vector<double> &Pixel::GetValues() const {
    return values_;
}

std::vector<double> &Pixel::GetValues() {
    return values_;
}

PixelArray::PixelArray(int32_t width, int32_t height, uint16_t bits_per_pixel, uint8_t padding,
                       const std::vector<std::vector<Pixel>> &matrix)
    : width_(width), height_(height), bits_per_pixel_(bits_per_pixel), padding_(padding), matrix_(matrix) {
}

int32_t PixelArray::GetWidth() const {
    return width_;
}

int32_t PixelArray::GetHeight() const {
    return height_;
}

uint16_t PixelArray::GetBPP() const {
    return bits_per_pixel_;
}

uint8_t PixelArray::GetPadding() const {
    return padding_;
}

const std::vector<std::vector<Pixel>> &PixelArray::GetMatrix() const {
    return matrix_;
}

std::vector<std::vector<Pixel>> &PixelArray::GetMatrix() {
    return matrix_;
}

void PixelArray::Save(std::ofstream &os) const {
    const size_t channels = static_cast<size_t>(bits_per_pixel_) / BitsInBytes;
    for (int32_t i = 0; i < height_; ++i) {
        for (int32_t j = 0; j < width_; ++j) {
            const auto &values = matrix_[static_cast<size_t>(i)][static_cast<size_t>(j)].GetValues();
            for (size_t k = 0; k < channels; ++k) {
                WriteByte(os, FromDoubleToUnsignedChar(values[k]));
            }
        }
        for (int32_t p = 0; p < padding_; ++p) {
            WriteImpl(os, static_cast<char>(0));
        }
    }
}

BMPHeader::BMPHeader(uint32_t file_size, int32_t offset) : file_size_(file_size), offset_(offset) {
}

void BMPHeader::Save(std::ofstream &os) const {
    WriteImpl(os, identifier_);
    WriteImpl(os, file_size_);
    WriteImpl(os, first_reserved_);
    WriteImpl(os, second_reserved_);
    WriteImpl(os, offset_);
}

DIBHeader::DIBHeader(int32_t width, int32_t height, uint16_t bits_per_pixel, uint32_t image_size)
    : width_(width), height_(height), bits_per_pixel_(bits_per_pixel), image_size_(image_size) {
}

int32_t DIBHeader::GetWidth() const {
    return width_;
}

int32_t DIBHeader::GetHeight() const {
    return height_;
}

uint16_t DIBHeader::GetBPP() const {
    return bits_per_pixel_;
}

void DIBHeader::Save(std::ofstream &os) const {
    WriteImpl(os, header_size_);
    WriteImpl(os, width_);
    WriteImpl(os, height_);
    WriteImpl(os, planes_);
    WriteImpl(os, bits_per_pixel_);
    WriteImpl(os, compression_);
    WriteImpl(os, image_size_);
    WriteImpl(os, horizontal_resolution_);
    WriteImpl(os, vertical_resolution_);
    WriteImpl(os, colors_);
    WriteImpl(os, important_colors_);
}

BmpFile::BmpFile(const PixelArray &pixel_array) : pixel_array_(pixel_array) {
    const int32_t width = pixel_array.GetWidth();
    const int32_t height = pixel_array.GetHeight();
    const uint16_t bits_per_pixel = pixel_array.GetBPP();
    const uint8_t padding = pixel_array.GetPadding();

    const uint32_t bytes_per_pixel = static_cast<uint32_t>(bits_per_pixel) / BitsInBytes;
    const uint32_t row_size = static_cast<uint32_t>(width) * bytes_per_pixel + static_cast<uint32_t>(padding);
    const uint32_t pixel_data_size = row_size * static_cast<uint32_t>(height);

    dib_header_ = DIBHeader(width, height, bits_per_pixel, pixel_data_size);
    const uint32_t file_size = SizeOfBMPHeaderInBytes + SizeOfDIBHeaderInBytes + pixel_data_size;
    bmp_header_ = BMPHeader(file_size, SizeOfBMPHeaderInBytes + SizeOfDIBHeaderInBytes);
}

BmpFile::BmpFile(const BMPHeader &header, const DIBHeader &dib_header, const PixelArray &pixel_array)
    : bmp_header_(header), dib_header_(dib_header), pixel_array_(pixel_array) {
}

PixelArray BmpFile::GetPixelArray() const {
    return pixel_array_;
}

void BmpFile::Save(std::ofstream &os) const {
    bmp_header_.Save(os);
    dib_header_.Save(os);
    pixel_array_.Save(os);
}

BMPHeader ReadBMPHeader(std::ifstream &input_stream) {
    char type[2] = {};
    ReadImpl(input_stream, type[0]);
    ReadImpl(input_stream, type[1]);

    if (type[0] != 'B' || type[1] != 'M') {
        std::cerr << "Error: Not a BMP file!" << std::endl;
        std::exit(1);
    }

    uint32_t file_size = 0;
    int16_t first_reserved = 0;
    int16_t second_reserved = 0;
    int32_t offset = 0;

    ReadImpl(input_stream, file_size);
    ReadImpl(input_stream, first_reserved);
    ReadImpl(input_stream, second_reserved);
    ReadImpl(input_stream, offset);

    return BMPHeader(file_size, offset);
}

DIBHeader ReadDIBHeader(std::ifstream &input_stream) {
    uint32_t header_size = 0;
    int32_t width = 0;
    int32_t height = 0;
    uint16_t planes = 0;
    uint16_t bits_per_pixel = 0;
    uint32_t compression = 0;
    uint32_t image_size = 0;
    int32_t horizontal_resolution = 0;
    int32_t vertical_resolution = 0;
    uint32_t colors = 0;
    uint32_t important_colors = 0;

    ReadImpl(input_stream, header_size);
    ReadImpl(input_stream, width);
    ReadImpl(input_stream, height);
    ReadImpl(input_stream, planes);
    ReadImpl(input_stream, bits_per_pixel);
    ReadImpl(input_stream, compression);
    ReadImpl(input_stream, image_size);
    ReadImpl(input_stream, horizontal_resolution);
    ReadImpl(input_stream, vertical_resolution);
    ReadImpl(input_stream, colors);
    ReadImpl(input_stream, important_colors);

    return DIBHeader(width, height, bits_per_pixel, image_size);
}

PixelArray ReadPixelArray(std::ifstream &input_stream, uint16_t bits_per_pixel, int32_t width, int32_t height) {
    const size_t channels = static_cast<size_t>(bits_per_pixel) / BitsInBytes;
    const size_t row_bytes = static_cast<size_t>(width) * channels;
    const uint8_t padding = static_cast<uint8_t>((row_bytes + 3) / 4 * 4 - row_bytes);

    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(height),
                                           std::vector<Pixel>(static_cast<size_t>(width), Pixel(bits_per_pixel)));
    std::vector<unsigned char> buffer(row_bytes);

    for (int32_t i = 0; i < height; ++i) {
        input_stream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(row_bytes));

        for (int32_t j = 0; j < width; ++j) {
            std::vector<double> values(channels);
            for (size_t c = 0; c < channels; ++c) {
                values[c] = FromUnsignedCharToDouble(buffer[static_cast<size_t>(j) * channels + c]);
            }
            matrix[static_cast<size_t>(i)][static_cast<size_t>(j)] = Pixel(bits_per_pixel, values);
        }

        input_stream.ignore(padding);
    }

    return PixelArray(width, height, bits_per_pixel, padding, matrix);
}

BmpFile LoadBmp(const std::string &file_path) {
    std::ifstream input_stream(file_path, std::ios::binary);
    if (!input_stream.is_open()) {
        throw std::runtime_error("Cannot open file: " + file_path);
    }

    const auto bmp_header = ReadBMPHeader(input_stream);
    const auto dib_header = ReadDIBHeader(input_stream);
    const auto pixel_array =
        ReadPixelArray(input_stream, dib_header.GetBPP(), dib_header.GetWidth(), dib_header.GetHeight());

    return BmpFile(bmp_header, dib_header, pixel_array);
}

void SaveBmp(const std::string &file_path, const BmpFile &bmp_file) {
    std::ofstream output_stream(file_path, std::ios::binary);
    bmp_file.Save(output_stream);
}