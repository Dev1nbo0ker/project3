#pragma once
#include <string>
#include <opencv2/opencv.hpp>

enum class Algorithm { Huffman, RLE, LZW, DCT };

namespace Compressor {
void compressImage(const std::string &algoName, const cv::Mat &img, const std::string &outputPath, int quality = 75);
}
