#include <iostream>
#include <chrono>
#include <filesystem>
#include "core/ImageData.h"
#include "core/Compressor.h"
#include "core/Decompressor.h"

// CLI entry point. Usage examples printed when args mismatch.
// 中文说明：命令行入口，主要负责解析用户输入并调用压缩/解压逻辑。

void printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  img_compress <algo> compress <input> <output> [quality]\n";
    std::cout << "  img_compress <algo> decompress <input> <output>\n";
    std::cout << "Algo: huffman | rle | lzw | dct (dct requires quality 1-100 on compress)\n";
}

int main(int argc, char **argv) {
    // 参数数量不足时直接输出帮助信息，避免后续访问越界。
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
        // 根据模式决定执行压缩还是解压，两条路径共享同一套异常处理。
        if (mode == "compress") {
            auto img = ImageIO::loadImage(input, false);
            // 记录耗时与压缩率，方便用户评估算法效果。
            auto start = std::chrono::steady_clock::now();
            Compressor::compressImage(algo, img, output, quality);
            auto end = std::chrono::steady_clock::now();
            auto originalSize = static_cast<uint64_t>(img.total() * img.elemSize());
            auto compressedSize = std::filesystem::file_size(output);
            double ratio = compressedSize ? static_cast<double>(originalSize) / compressedSize : 0.0;
            std::cout << "Compression done. Ratio=" << ratio << ", time(ms)="
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
        } else if (mode == "decompress") {
            // 解压路径：读取压缩文件后立即写出图像，记录耗时反馈给用户。
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
