#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "ImageData.h"

namespace DCTCodec {
struct QuantBlock { // store 64 coefficients
    int16_t coeffs[64];
};

void compress(const cv::Mat &img, const std::string &outputPath, int quality);
cv::Mat decompress(const std::string &inputPath);
}
