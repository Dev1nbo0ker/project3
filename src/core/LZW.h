#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "ImageData.h"

namespace LZW {
std::vector<uint16_t> encodeChannel(const std::vector<uint8_t> &data);
std::vector<uint8_t> decodeChannel(const std::vector<uint16_t> &codes);
void compress(const cv::Mat &img, const std::string &outputPath);
cv::Mat decompress(const std::string &inputPath);
}
