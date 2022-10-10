#include "Encoder.h"
#include <iostream>
#include <chrono>

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

    std::cout << "Total encoding time: " << duration.count() << " milliseconds." << std::endl;

    return 0;
}
