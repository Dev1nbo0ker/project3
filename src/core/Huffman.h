#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <string>
#include <array>
#include "ImageData.h"
#include "BitIO.h"

struct HuffmanNode {
    int value; // -1 for internal
    uint64_t freq;
    HuffmanNode *left;
    HuffmanNode *right;
};

// Huffman coding for byte streams.
namespace Huffman {
std::vector<uint8_t> compressChannel(const std::vector<uint8_t> &data, uint64_t &validBits, std::array<uint64_t,256> &freqOut);
std::vector<uint8_t> decompressChannel(const std::vector<uint8_t> &encoded, uint64_t validBits, const std::array<uint64_t,256> &freq);

void compress(const cv::Mat &img, const std::string &outputPath);
cv::Mat decompress(const std::string &inputPath);
}
