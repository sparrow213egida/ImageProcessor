#include "bmp_structures.h"
#include "filters.h"

#include <iostream>
#include <stdexcept>
#include <string>

#ifndef TESTING

int main(int argc, char** argv) {
    if (argc < 3) {
        PrintHelp();
        return 0;
    }

    try {
        const BmpFile bmp = LoadBmp(argv[1]);
        PixelArray current = bmp.GetPixelArray();

        for (int i = 3; i < argc; ++i) {
            const std::string filter = argv[i];

            if (filter == "-crop") {
                if (i + 2 >= argc) {
                    std::cerr << "Error: -crop requires width and height parameters\n";
                    return 1;
                }
                const int32_t width = std::stoi(argv[++i]);
                const int32_t height = std::stoi(argv[++i]);
                current = CropFilter::Apply(current, width, height);
            } else if (filter == "-gs") {
                current = GrayScaleFilter::Apply(current);
            } else if (filter == "-neg") {
                current = NegativeFilter::Apply(current);
            } else if (filter == "-sharp") {
                current = SharpeningFilter::Apply(current);
            } else if (filter == "-edge") {
                if (i + 1 >= argc) {
                    std::cerr << "Error: -edge requires threshold parameter\n";
                    return 1;
                }
                const double threshold = std::stod(argv[++i]);
                current = EdgeDetectionFilter::Apply(current, threshold);
            } else if (filter == "-blur") {
                if (i + 1 >= argc) {
                    std::cerr << "Error: -blur requires sigma parameter\n";
                    return 1;
                }
                const double sigma = std::stod(argv[++i]);
                current = GaussianBlurFilter::Apply(current, sigma);
            } else {
                std::cerr << "Unknown filter: " << filter << '\n';
                PrintHelp();
                return 1;
            }
        }

        SaveBmp(argv[2], BmpFile(current));
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
#endif