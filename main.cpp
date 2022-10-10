#include "Encoder.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <string>

int getFileSizeInBytes(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    const auto begin = f.tellg();
    f.seekg (0, std::ios::end);
    const auto end = f.tellg();
    return (int)(end - begin);
}

int main(int argc, char *argv[]) {
    /* Input validation */

    if (argc < 3) {
        std::cout << "Input and output file paths must be provided." << std::endl;
        return -1;
    }

    std::string inPath = argv[1];
    std::string outPath = argv[2];

    /* Encoding */

    Encoder encoder;

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        encoder.readImagePNG(inPath);
    } catch (...) {
        std::cout << "File could not be read." << std::endl;
    }

    encoder.convertColorspace();
    encoder.createPaddedImage();
    encoder.generateBlocks();
    encoder.transformBlocksWithDCT();
    encoder.quantizeBlocks();
    encoder.zigZagVectorizeBlocks();
    encoder.writeJPEG(outPath);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Total encoding time: " << duration.count() << " ms" << std::endl;

    /* Calculating compression ratio */

    const int uncompressedSizeInBytes = encoder.width * encoder.height * 3;
    const int compressedSizeInBytes = getFileSizeInBytes(outPath);

    std::cout << std::endl;
    std::cout << "Uncompressed image size: " << uncompressedSizeInBytes << " bytes" << std::endl;
    std::cout << "Compressed image size: " << compressedSizeInBytes << " bytes" << std::endl;
    std::cout << "Compression ratio: " << (double)uncompressedSizeInBytes / (double)compressedSizeInBytes << std::endl;

    return 0;
}
