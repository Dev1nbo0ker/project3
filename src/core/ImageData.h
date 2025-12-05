#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// Helper for reading/writing images and flattening channels.
struct ImageData {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t channels = 0; // 1 or 3
    std::vector<std::vector<uint8_t>> channelData; // per-channel byte array
};

namespace ImageIO {
cv::Mat loadImage(const std::string &path, bool forceColor);
void saveImage(const std::string &path, const cv::Mat &img);
ImageData fromMat(const cv::Mat &img);
cv::Mat toMat(const ImageData &data);
}
