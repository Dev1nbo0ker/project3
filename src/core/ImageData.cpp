#include "ImageData.h"
#include <stdexcept>
#include <cstring>

cv::Mat ImageIO::loadImage(const std::string &path, bool forceColor) {
    cv::Mat img = cv::imread(path, forceColor ? cv::IMREAD_COLOR : cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        throw std::runtime_error("Failed to read image: " + path);
    }
    if (img.channels() == 4) {
        cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
    }
    if (img.channels() == 1 || img.channels() == 3) {
        return img;
    }
    throw std::runtime_error("Unsupported channel count: " + std::to_string(img.channels()));
}

void ImageIO::saveImage(const std::string &path, const cv::Mat &img) {
    if (!cv::imwrite(path, img)) {
        throw std::runtime_error("Failed to write image: " + path);
    }
}

ImageData ImageIO::fromMat(const cv::Mat &img) {
    ImageData data;
    data.width = static_cast<uint32_t>(img.cols);
    data.height = static_cast<uint32_t>(img.rows);
    data.channels = static_cast<uint8_t>(img.channels());
    std::vector<cv::Mat> planes;
    if (data.channels == 3) {
        cv::split(img, planes);
    } else {
        planes.push_back(img);
    }
    data.channelData.resize(planes.size());
    for (size_t i = 0; i < planes.size(); ++i) {
        data.channelData[i].assign(planes[i].datastart, planes[i].dataend);
    }
    return data;
}

cv::Mat ImageIO::toMat(const ImageData &data) {
    std::vector<cv::Mat> planes;
    planes.reserve(data.channelData.size());
    for (const auto &vec : data.channelData) {
        cv::Mat plane(static_cast<int>(data.height), static_cast<int>(data.width), CV_8UC1);
        std::memcpy(plane.data, vec.data(), vec.size());
        planes.push_back(plane);
    }
    if (data.channels == 3) {
        cv::Mat merged;
        cv::merge(planes, merged);
        return merged;
    }
    return planes[0];
}
