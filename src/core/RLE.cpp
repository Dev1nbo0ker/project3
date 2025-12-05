#include "RLE.h"
#include <fstream>
#include <stdexcept>

// Simple RLE: (value, run_length uint16_t)
std::vector<uint8_t> RLE::encodeChannel(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> out;
    size_t i = 0;
    while (i < data.size()) {
        uint8_t val = data[i];
        uint16_t run = 1;
        while (i + run < data.size() && data[i + run] == val && run < 0xFFFF) {
            ++run;
        }
        out.push_back(val);
        out.push_back(static_cast<uint8_t>(run >> 8));
        out.push_back(static_cast<uint8_t>(run & 0xFF));
        i += run;
    }
    return out;
}

std::vector<uint8_t> RLE::decodeChannel(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i + 2 < data.size(); i += 3) {
        uint8_t val = data[i];
        uint16_t run = static_cast<uint16_t>(data[i+1] << 8 | data[i+2]);
        out.insert(out.end(), run, val);
    }
    return out;
}

void RLE::compress(const cv::Mat &img, const std::string &outputPath) {
    ImageData data = ImageIO::fromMat(img);
    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file");
    ofs.write("RLE ", 4);
    ofs.write(reinterpret_cast<const char*>(&data.width), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char*>(&data.height), sizeof(uint32_t));
    ofs.put(static_cast<char>(data.channels));
    ofs.put(0); ofs.put(0); ofs.put(0);
    for (const auto &ch : data.channelData) {
        auto encoded = encodeChannel(ch);
        uint32_t sz = static_cast<uint32_t>(encoded.size());
        ofs.write(reinterpret_cast<const char*>(&sz), sizeof(uint32_t));
        ofs.write(reinterpret_cast<const char*>(encoded.data()), encoded.size());
    }
}

cv::Mat RLE::decompress(const std::string &inputPath) {
    std::ifstream ifs(inputPath, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file");
    char magic[4];
    ifs.read(magic, 4);
    if (std::string(magic,4) != "RLE ") throw std::runtime_error("Invalid magic for RLE");
    ImageData data;
    ifs.read(reinterpret_cast<char*>(&data.width), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&data.height), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&data.channels), 1);
    char pad[3]; ifs.read(pad, 3);
    data.channelData.resize(data.channels);
    for (auto &ch : data.channelData) {
        uint32_t sz = 0;
        ifs.read(reinterpret_cast<char*>(&sz), sizeof(uint32_t));
        std::vector<uint8_t> enc(sz);
        ifs.read(reinterpret_cast<char*>(enc.data()), sz);
        ch = decodeChannel(enc);
    }
    return ImageIO::toMat(data);
}
