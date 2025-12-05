#pragma once
#include <string>
#include <opencv2/opencv.hpp>

namespace Decompressor {
cv::Mat decompressImage(const std::string &algoName, const std::string &inputPath);
}
