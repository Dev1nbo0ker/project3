#include <iostream>
#include <chrono>
#include <filesystem>
#include "core/ImageData.h"
#include "core/Compressor.h"
#include "core/Decompressor.h"

// CLI entry point. Usage examples printed when args mismatch.

void printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  img_compress <algo> compress <input> <output> [quality]\n";
    std::cout << "  img_compress <algo> decompress <input> <output>\n";
    std::cout << "Algo: huffman | rle | lzw | dct (dct requires quality 1-100 on compress)\n";
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printUsage();
        return 1;
    }
    std::string algo = argv[1];
    std::string mode = argv[2];
    std::string input = argv[3];
    std::string output = argv[4];
    int quality = 75;
    if (mode == "compress" && algo == "dct" && argc >= 6) {
        quality = std::stoi(argv[5]);
    }
    try {
        if (mode == "compress") {
            auto img = ImageIO::loadImage(input, algo != "dct");
            auto start = std::chrono::steady_clock::now();
            Compressor::compressImage(algo, img, output, quality);
            auto end = std::chrono::steady_clock::now();
            auto originalSize = static_cast<uint64_t>(img.total() * img.elemSize());
            auto compressedSize = std::filesystem::file_size(output);
            double ratio = compressedSize ? static_cast<double>(originalSize) / compressedSize : 0.0;
            std::cout << "Compression done. Ratio=" << ratio << ", time(ms)="
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
        } else if (mode == "decompress") {
            auto start = std::chrono::steady_clock::now();
            auto img = Decompressor::decompressImage(algo, input);
            auto end = std::chrono::steady_clock::now();
            ImageIO::saveImage(output, img);
            std::cout << "Decompression done. time(ms)="
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
        } else {
            printUsage();
            return 1;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
