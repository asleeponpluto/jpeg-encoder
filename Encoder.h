#pragma once

#include <cstdint>
#include <vector>
#include <array>

class Encoder {
public:
    struct RGB {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        explicit RGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) {
            this->r = r;
            this->g = g;
            this->b = b;
        }
    };

    struct YCbCr {
        uint8_t y;
        uint8_t cb;
        uint8_t cr;

        explicit YCbCr(uint8_t y = 0, uint8_t cb = 0, uint8_t cr = 0) {
            this->y = y;
            this->cb = cb;
            this->cr = cr;
        }
    };

    struct Block {
        std::array<int, 64> y{};
        std::array<int, 64> cb{};
        std::array<int, 64> cr{};
    };

    enum PixelType { Luminance, Chrominance };

    const static int LuminanceQuantizationTable[64];
    const static int ChrominanceQuantizationTable[64];
    const static int ZigZagTable[64];

    int width;
    int height;
    int paddedWidth;
    int paddedHeight;
    std::array<double, 64> cosineTable{};

    std::vector<RGB> imageRGB;
    std::vector<YCbCr> imageYCbCr;
    std::vector<YCbCr> paddedYCbCr;
    std::vector<Block> blocks;

    static int round(double num);
    static int clamp(int num, int low, int high);
    static int min(int x, int y);
    static unsigned int getIndex(unsigned int x, unsigned int y, unsigned int width);

    static YCbCr RGBToYCbCr(const RGB& in);
    void generateCosineTable();
    static double C(unsigned int i);
    int calcDCTCoefficient(unsigned int x, unsigned int y, const std::array<int, 64>& block);
    void transformBlockWithDCT(std::array<int, 64>& block);
    static void quantizeBlock(std::array<int, 64>& block, PixelType type);
    static void zigZagVectorizeBlock(std::array<int, 64>& block);
    static std::vector<int> runLengthEncodeBlockAC(const std::array<int, 64>& block); // unused (replicated in Writer)

public:
    Encoder();

    void readImagePNG(const std::string& path);
    void convertColorspace();
    void createPaddedImage();
    void generateBlocks();
    void transformBlocksWithDCT();
    void quantizeBlocks();
    void zigZagVectorizeBlocks();
    void writeJPEG(const std::string& path) const;
};