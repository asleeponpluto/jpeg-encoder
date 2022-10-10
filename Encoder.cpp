#define PI 3.14159265358979323846
#define STB_IMAGE_IMPLEMENTATION

#include "Encoder.h"
#include "Writer.h"
#include "stb_image.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>

const int Encoder::LuminanceQuantizationTable[64] = {
        16, 11, 10, 16,  24,  40,  51,  61,
        12, 12, 14, 19,  26,  58,  60,  55,
        14, 13, 16, 24,  40,  57,  69,  56,
        14, 17, 22, 29,  51,  87,  80,  62,
        18, 22, 37, 56,  68, 109, 103,  77,
        24, 36, 55, 64,  81, 104, 113,  92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103,  99
};

const int Encoder::ChrominanceQuantizationTable[64] = {
        17, 18, 24, 47, 99, 99, 99, 99,
        18, 21, 26, 66, 99, 99, 99, 99,
        24, 26, 56, 99, 99, 99, 99, 99,
        47, 66, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99
};

const int Encoder::ZigZagTable[64] = {
         0,  1,  8, 16,  9,  2,  3, 10,     //  0,  1,  5,  6, 14, 15, 27, 28,
        17, 24, 32, 25, 18, 11,  4,  5,     //  2,  4,  7, 13, 16, 26, 29, 42,
        12, 19, 26, 33, 40, 48, 41, 34,     //  3,  8, 12, 17, 25, 30, 41, 43,
        27, 20, 13,  6,  7, 14, 21, 28,     //  9, 11, 18, 24, 31, 40, 44, 53,
        35, 42, 49, 56, 57, 50, 43, 36,     // 10, 19, 23, 32, 39, 45, 52, 54,
        29, 22, 15, 23, 30, 37, 44, 51,     // 20, 22, 33, 38, 46, 51, 55, 60,
        58, 59, 52, 45, 38, 31, 39, 46,     // 21, 34, 37, 47, 50, 56, 59, 61,
        53, 60, 61, 54, 47, 55, 62, 63      // 35, 36, 48, 49, 57, 58, 62, 63
};

Encoder::Encoder() {
    generateCosineTable();
}

int Encoder::round(const double num) {
    return floor(num + 0.5);
}

int Encoder::clamp(const int num, const int low, const int high) {
    return std::max(low, std::min(num, high));
}

int Encoder::min(int a, int b) {
    return (b < a) ? b : a;
}

unsigned int Encoder::getIndex(unsigned int x, unsigned int y, unsigned int width) {
    return x + (y * width);
}

Encoder::YCbCr Encoder::RGBToYCbCr(const Encoder::RGB& in) {
    YCbCr out;

    out.y = clamp(round((0.299 * in.r) + (0.587 * in.g) + (0.114 * in.b)), 0, 255);
    out.cb = clamp(round((-0.168935 * in.r) + (-0.331665 * in.g) + (0.50059 * in.b) + 128), 0, 255);
    out.cr = clamp(round((0.499813 * in.r) + (-0.418531 * in.g) + (-0.081282 * in.b) + 128), 0, 255);

    return out;
}

void Encoder::generateCosineTable() {
    for (unsigned int i = 0; i < 8; i++) {
        for (unsigned int j = 0; j < 8; j++) {
            cosineTable[getIndex(i, j, 8)] = std::cos((double)((2*i+1)*j*PI)/(double)(2*8));
        }
    }
}

double Encoder::C(unsigned int x) {
    static const double inverseSqrtOfTwo = (double)1 / sqrt(2);

    if (x == 0)
        return inverseSqrtOfTwo;

    return 1;
}

int Encoder::calcDCTCoefficient(unsigned int i, unsigned int j, const std::array<int, 64>& block) {
    static const double inverseSqrtOfSixteen = (double)1 / sqrt(16);
    double temp = 0;

    for (unsigned int x = 0; x < 8; x++) {
        for (unsigned int y = 0; y < 8; y++) {
            temp += (block[getIndex(x, y, 8)] - 128)
                    * cosineTable[getIndex(x, i, 8)]
                    * cosineTable[getIndex(y, j, 8)];
        }
    }

    temp *= inverseSqrtOfSixteen * C(i) * C(j);

    return round(temp);
}

void Encoder::transformBlockWithDCT(std::array<int, 64> &block) {
    std::array<int, 64> transformed{};

    for (unsigned int i = 0; i < 8; i++) {
        for (unsigned int j = 0; j < 8; j++) {
            transformed[getIndex(i, j, 8)] = calcDCTCoefficient(i, j, block);
        }
    }

    block = transformed;
}

void Encoder::quantizeBlock(std::array<int, 64> &block, PixelType type) {
    std::array<int, 64> quantized{};
    const int* table = type == Luminance ? LuminanceQuantizationTable : ChrominanceQuantizationTable;

    for (unsigned int i = 0; i < 8; i++) {
        for (unsigned int j = 0; j < 8; j++) {
            quantized[getIndex(i, j, 8)] = round(
                    (double)block[getIndex(i, j, 8)] / (double)table[getIndex(i, j, 8)]);
        }
    }

    block = quantized;
}

void Encoder::zigZagVectorizeBlock(std::array<int, 64> &block) {
    std::array<int, 64> zigZagVector{};

    for (unsigned int i = 0; i < 64; i++) {
        zigZagVector[i] = block[ZigZagTable[i]];
    }

    block = zigZagVector;
}

std::vector<int> Encoder::runLengthEncodeBlockAC(const std::array<int, 64> &block) {
    std::vector<int> rle;
    int currZeroCount = 0;

    for (unsigned int i = 1; i < 64; i++) {
        if (block[i] == 0) {
            currZeroCount++;
            continue;
        }

        while (currZeroCount > 15) {
            rle.push_back(15);
            rle.push_back(0);
            currZeroCount -= 16;
        }

        rle.push_back(currZeroCount);
        rle.push_back(block[i]);
        currZeroCount = 0;
    }

    rle.push_back(0);
    rle.push_back(0);

    return rle;
}

void Encoder::readImagePNG(const std::string &path) {
    uint8_t* image = stbi_load(path.c_str(), &width, &height, nullptr, 3);

    if (image == nullptr)
        throw std::invalid_argument("Couldn't find file at path: " + path);

    for (unsigned int i = 0; i < 3 * width * height; i += 3) {
        imageRGB.emplace_back(image[i], image[i+1], image[i+2]);
    }

    stbi_image_free(image);
}

void Encoder::convertColorspace() {
    for (const RGB& rgb : imageRGB) {
        imageYCbCr.emplace_back(RGBToYCbCr(rgb));
    }
}

void Encoder::createPaddedImage() {
    paddedWidth = width % 8 == 0 ? width : width + (8 - (width % 8));
    paddedHeight = height % 8 == 0 ? height : height + (8 - (height % 8));

    for (int j = 0; j < paddedHeight; j++) {
        for (int i = 0; i < paddedWidth; i++) {
            YCbCr pixel = imageYCbCr[getIndex(min(i, width - 1), min(j, height - 1), width)];
            paddedYCbCr.push_back(pixel);
        }
    }
}

void Encoder::generateBlocks() {
    for (int mcuY = 0; mcuY < paddedHeight; mcuY += 8) {
        for (int mcuX = 0; mcuX < paddedWidth; mcuX += 8) {
            Block block;

            for (int y = mcuY; y < mcuY + 8; y++) {
                for (int x = mcuX; x < mcuX + 8; x++) {
                    block.y[getIndex(x - mcuX, y - mcuY, 8)] = paddedYCbCr[getIndex(x, y, paddedWidth)].y;
                    block.cb[getIndex(x - mcuX, y - mcuY, 8)] = paddedYCbCr[getIndex(x, y, paddedWidth)].cb;
                    block.cr[getIndex(x - mcuX, y - mcuY, 8)] = paddedYCbCr[getIndex(x, y, paddedWidth)].cr;
                }
            }

            blocks.push_back(block);
        }
    }
}

void Encoder::transformBlocksWithDCT() {
    for (Block& block : blocks) {
        transformBlockWithDCT(block.y);
        transformBlockWithDCT(block.cb);
        transformBlockWithDCT(block.cr);
    }

    std::cout << "Discrete Cosine Transform ran on " << blocks.size() << " blocks." << std::endl;
}

void Encoder::quantizeBlocks() {
    for (Block& block : blocks) {
        quantizeBlock(block.y, Luminance);
        quantizeBlock(block.cb, Chrominance);
        quantizeBlock(block.cr, Chrominance);
    }
}

void Encoder::zigZagVectorizeBlocks() {
    for (Block& block : blocks) {
        zigZagVectorizeBlock(block.y);
        zigZagVectorizeBlock(block.cb);
        zigZagVectorizeBlock(block.cr);
    }
}

void Encoder::writeJPEG(const std::string &path) const {
    std::ofstream wf(path, std::ios::out | std::ios::binary);

    if (!wf) {
        std::cout << "Failed to open file to write!" << std::endl;
        return;
    }

    TooJpeg::writeJpeg(wf, blocks, width, height);

    wf.close();
}

