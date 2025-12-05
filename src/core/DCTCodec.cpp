#include "DCTCodec.h"
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
const int N = 8;
// JPEG-like luminance base matrix
const int baseQ[8][8] = {
    {16,11,10,16,24,40,51,61},
    {12,12,14,19,26,58,60,55},
    {14,13,16,24,40,57,69,56},
    {14,17,22,29,51,87,80,62},
    {18,22,37,56,68,109,103,77},
    {24,35,55,64,81,104,113,92},
    {49,64,78,87,103,121,120,101},
    {72,92,95,98,112,100,103,99}
};

double alpha(int u) { return u == 0 ? std::sqrt(1.0 / N) : std::sqrt(2.0 / N); }

void dct8x8(const double in[8][8], double out[8][8]) {
    for (int u = 0; u < N; ++u) {
        for (int v = 0; v < N; ++v) {
            double sum = 0.0;
            for (int x = 0; x < N; ++x) {
                for (int y = 0; y < N; ++y) {
                    sum += in[x][y] *
                        std::cos(((2 * x + 1) * u * M_PI) / (2 * N)) *
                        std::cos(((2 * y + 1) * v * M_PI) / (2 * N));
                }
            }
            out[u][v] = alpha(u) * alpha(v) * sum;
        }
    }
}

void idct8x8(const double in[8][8], double out[8][8]) {
    for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
            double sum = 0.0;
            for (int u = 0; u < N; ++u) {
                for (int v = 0; v < N; ++v) {
                    sum += alpha(u) * alpha(v) * in[u][v] *
                        std::cos(((2 * x + 1) * u * M_PI) / (2 * N)) *
                        std::cos(((2 * y + 1) * v * M_PI) / (2 * N));
                }
            }
            out[x][y] = sum;
        }
    }
}

void buildQuantMatrix(int quality, double q[8][8]) {
    int qf = std::max(1, std::min(quality, 100));
    double scale = (qf < 50) ? 50.0 / qf : (200.0 - 2 * qf) / 100.0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            q[i][j] = baseQ[i][j] * scale;
        }
    }
}
}

void DCTCodec::compress(const cv::Mat &img, const std::string &outputPath, int quality) {
    cv::Mat gray;
    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img.clone();
    }
    if (gray.channels() != 1) throw std::runtime_error("DCT only supports 1 channel");

    uint32_t width = static_cast<uint32_t>(gray.cols);
    uint32_t height = static_cast<uint32_t>(gray.rows);
    int paddedW = (width + 7) / 8 * 8;
    int paddedH = (height + 7) / 8 * 8;
    cv::Mat padded;
    cv::copyMakeBorder(gray, padded, 0, paddedH - gray.rows, 0, paddedW - gray.cols, cv::BORDER_REPLICATE);

    double qmat[8][8];
    buildQuantMatrix(quality, qmat);

    std::vector<QuantBlock> blocks;
    for (int y = 0; y < paddedH; y += 8) {
        for (int x = 0; x < paddedW; x += 8) {
            double block[8][8];
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    block[i][j] = padded.at<uint8_t>(y + i, x + j) - 128.0; // shift center
                }
            }
            double freq[8][8];
            dct8x8(block, freq);
            QuantBlock qb{};
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    double q = qmat[i][j];
                    qb.coeffs[i*8 + j] = static_cast<int16_t>(std::round(freq[i][j] / q));
                }
            }
            blocks.push_back(qb);
        }
    }

    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file");
    ofs.write("DCT ", 4);
    ofs.write(reinterpret_cast<const char*>(&width), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char*>(&height), sizeof(uint32_t));
    uint8_t channels = 1;
    ofs.put(static_cast<char>(channels));
    ofs.put(0); ofs.put(0); ofs.put(0);
    uint8_t qByte = static_cast<uint8_t>(std::max(1, std::min(quality, 100)));
    ofs.put(static_cast<char>(qByte));
    ofs.put(0); ofs.put(0); ofs.put(0); // pad
    uint32_t padW32 = static_cast<uint32_t>(paddedW);
    uint32_t padH32 = static_cast<uint32_t>(paddedH);
    ofs.write(reinterpret_cast<const char*>(&padW32), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char*>(&padH32), sizeof(uint32_t));
    for (const auto &qb : blocks) {
        ofs.write(reinterpret_cast<const char*>(qb.coeffs), sizeof(int16_t) * 64);
    }
}

cv::Mat DCTCodec::decompress(const std::string &inputPath) {
    std::ifstream ifs(inputPath, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file");
    char magic[4]; ifs.read(magic,4);
    if (std::string(magic,4) != "DCT ") throw std::runtime_error("Invalid magic for DCT");
    uint32_t width = 0, height = 0; uint8_t channels = 0;
    ifs.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&channels),1);
    char pad[3]; ifs.read(pad,3);
    if (channels != 1) throw std::runtime_error("DCT expects 1 channel");
    uint8_t qualityByte = 50; char pad2[3];
    ifs.read(reinterpret_cast<char*>(&qualityByte),1);
    ifs.read(pad2,3);
    uint32_t paddedW = 0, paddedH = 0;
    ifs.read(reinterpret_cast<char*>(&paddedW), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&paddedH), sizeof(uint32_t));
    double qmat[8][8];
    buildQuantMatrix(qualityByte, qmat);

    int blocksX = paddedW / 8;
    int blocksY = paddedH / 8;
    cv::Mat padded(paddedH, paddedW, CV_8UC1);
    for (int by = 0; by < blocksY; ++by) {
        for (int bx = 0; bx < blocksX; ++bx) {
            QuantBlock qb{};
            ifs.read(reinterpret_cast<char*>(qb.coeffs), sizeof(int16_t) * 64);
            double freq[8][8];
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    freq[i][j] = qb.coeffs[i*8 + j] * qmat[i][j];
                }
            }
            double spatial[8][8];
            idct8x8(freq, spatial);
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    int val = static_cast<int>(std::round(spatial[i][j] + 128.0));
                    val = std::max(0, std::min(255, val));
                    padded.at<uint8_t>(by*8 + i, bx*8 + j) = static_cast<uint8_t>(val);
                }
            }
        }
    }
    cv::Mat cropped = padded(cv::Rect(0,0,width,height)).clone();
    return cropped;
}
