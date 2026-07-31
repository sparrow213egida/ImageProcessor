#define TESTING

#include "bmp_structures.h"
#include "filters.h"

#include <gtest/gtest.h>
#include <vector>

namespace {

constexpr int32_t TestWidth1 = 1;
constexpr int32_t TestHeight1 = 1;
constexpr int32_t TestWidth3 = 3;
constexpr int32_t TestHeight3 = 3;
constexpr int32_t TestWidth4 = 4;
constexpr int32_t TestHeight4 = 4;
constexpr uint16_t TestBpp = 24;
constexpr uint8_t TestPadding = 0;

constexpr double TestRed = 0.8;
constexpr double TestGreen = 0.5;
constexpr double TestBlue = 0.2;
constexpr double TestGray = 0.5;

constexpr double RedCoeff = 0.299;
constexpr double GreenCoeff = 0.587;
constexpr double BlueCoeff = 0.114;

constexpr double TestThreshold = 0.5;
constexpr double Accuracy = 1e-6;

constexpr double FirstArgument = -0.1;
constexpr double SecondArgument = 1.2;

constexpr int MaxByte = 255;
constexpr int MidByte = 128;
constexpr double Half = 0.5;

constexpr double Zero = 0.0;
constexpr double One = 1.0;
constexpr double Two = 2.0;

}  // namespace

class ImageProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST_F(ImageProcessorTest, NumNormalizeWorks) {
    EXPECT_DOUBLE_EQ(NumNormalize(Half), Half);
    EXPECT_DOUBLE_EQ(NumNormalize(FirstArgument), Zero);
    EXPECT_DOUBLE_EQ(NumNormalize(SecondArgument), One);
}

TEST_F(ImageProcessorTest, UnsignedCharConversions) {
    EXPECT_DOUBLE_EQ(FromUnsignedCharToDouble(0), Zero);
    EXPECT_DOUBLE_EQ(FromUnsignedCharToDouble(MidByte), static_cast<double>(MidByte) / MaxByte);
    EXPECT_DOUBLE_EQ(FromUnsignedCharToDouble(MaxByte), One);

    EXPECT_EQ(FromDoubleToUnsignedChar(Zero), 0);
    EXPECT_EQ(FromDoubleToUnsignedChar(Half), MidByte);
    EXPECT_EQ(FromDoubleToUnsignedChar(One), MaxByte);
}

TEST_F(ImageProcessorTest, GrayscaleFilter) {
    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(TestHeight1),
                                           std::vector<Pixel>(static_cast<size_t>(TestWidth1), Pixel(TestBpp)));
    matrix[0][0] = Pixel(TestBpp, {TestBlue, TestGreen, TestRed});

    const PixelArray input(TestWidth1, TestHeight1, TestBpp, TestPadding, matrix);
    const PixelArray result = GrayScaleFilter::Apply(input);

    const double expected = RedCoeff * TestRed + GreenCoeff * TestGreen + BlueCoeff * TestBlue;
    const auto &values = result.GetMatrix()[0][0].GetValues();
    for (const auto &value : values) {
        EXPECT_DOUBLE_EQ(value, expected);
    }
}

TEST_F(ImageProcessorTest, NegativeFilter) {
    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(TestHeight1),
                                           std::vector<Pixel>(static_cast<size_t>(TestWidth1), Pixel(TestBpp)));
    matrix[0][0] = Pixel(TestBpp, {TestBlue, TestGreen, TestRed});

    const PixelArray input(TestWidth1, TestHeight1, TestBpp, TestPadding, matrix);
    const PixelArray result = NegativeFilter::Apply(input);

    const auto &values = result.GetMatrix()[0][0].GetValues();
    EXPECT_DOUBLE_EQ(values[0], One - TestBlue);
    EXPECT_DOUBLE_EQ(values[1], One - TestGreen);
    EXPECT_DOUBLE_EQ(values[2], One - TestRed);
}

TEST_F(ImageProcessorTest, SharpeningFilterConstantImage) {
    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(TestHeight3),
                                           std::vector<Pixel>(static_cast<size_t>(TestWidth3), Pixel(TestBpp)));
    for (auto &row : matrix) {
        for (auto &pixel : row) {
            pixel = Pixel(TestBpp, {TestGray, TestGray, TestGray});
        }
    }

    const PixelArray input(TestWidth3, TestHeight3, TestBpp, TestPadding, matrix);
    const PixelArray result = SharpeningFilter::Apply(input);

    for (const auto &row : result.GetMatrix()) {
        for (const auto &pixel : row) {
            for (const auto &value : pixel.GetValues()) {
                EXPECT_DOUBLE_EQ(value, TestGray);
            }
        }
    }
}

TEST_F(ImageProcessorTest, CropFilter) {
    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(TestHeight4),
                                           std::vector<Pixel>(static_cast<size_t>(TestWidth4), Pixel(TestBpp)));
    for (int32_t i = 0; i < TestHeight4; ++i) {
        for (int32_t j = 0; j < TestWidth4; ++j) {
            matrix[static_cast<size_t>(i)][static_cast<size_t>(j)] =
                Pixel(TestBpp, {static_cast<double>(i), static_cast<double>(j), Zero});
        }
    }

    const PixelArray input(TestWidth4, TestHeight4, TestBpp, TestPadding, matrix);
    const PixelArray result = CropFilter::Apply(input, 2, 2);

    EXPECT_EQ(result.GetWidth(), 2);
    EXPECT_EQ(result.GetHeight(), 2);

    const auto &values00 = result.GetMatrix()[0][0].GetValues();
    const auto &values11 = result.GetMatrix()[1][1].GetValues();
    EXPECT_DOUBLE_EQ(values00[0], Two);
    EXPECT_DOUBLE_EQ(values11[1], One);
}

TEST_F(ImageProcessorTest, GaussianBlurSmallSigma) {
    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(TestHeight3),
                                           std::vector<Pixel>(static_cast<size_t>(TestWidth3), Pixel(TestBpp)));
    for (auto &row : matrix) {
        for (auto &pixel : row) {
            pixel = Pixel(TestBpp, {TestGray, TestGray, TestGray});
        }
    }

    const PixelArray input(TestWidth3, TestHeight3, TestBpp, TestPadding, matrix);
    const PixelArray result = GaussianBlurFilter::Apply(input, 0.1);

    for (const auto &row : result.GetMatrix()) {
        for (const auto &pixel : row) {
            for (const auto &value : pixel.GetValues()) {
                EXPECT_NEAR(value, TestGray, Accuracy);
            }
        }
    }
}

TEST_F(ImageProcessorTest, EdgeDetection) {
    std::vector<std::vector<Pixel>> matrix(static_cast<size_t>(TestHeight3),
                                           std::vector<Pixel>(static_cast<size_t>(TestWidth3), Pixel(TestBpp)));
    for (int32_t i = 0; i < TestHeight3; ++i) {
        matrix[static_cast<size_t>(i)][0] = Pixel(TestBpp, {Zero, Zero, Zero});
        matrix[static_cast<size_t>(i)][1] = Pixel(TestBpp, {Zero, Zero, Zero});
        matrix[static_cast<size_t>(i)][2] = Pixel(TestBpp, {One, One, One});
    }

    const PixelArray input(TestWidth3, TestHeight3, TestBpp, TestPadding, matrix);
    const PixelArray result = EdgeDetectionFilter::Apply(input, TestThreshold);

    const auto &center = result.GetMatrix()[1][1].GetValues();
    EXPECT_DOUBLE_EQ(center[0], One);
    EXPECT_DOUBLE_EQ(center[1], One);
    EXPECT_DOUBLE_EQ(center[2], One);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}