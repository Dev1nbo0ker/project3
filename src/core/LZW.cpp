#include "LZW.h"
#include <unordered_map>
#include <fstream>
#include <stdexcept>

// Basic LZW with 16-bit codes. 字典大小限制4096，简单易懂。
std::vector<uint16_t> LZW::encodeChannel(const std::vector<uint8_t> &data) {
    std::unordered_map<std::string, uint16_t> dict;
    for (int i = 0; i < 256; ++i) {
        dict[std::string(1, static_cast<char>(i))] = static_cast<uint16_t>(i);
    }
    uint16_t dictSize = 256;
    const uint16_t dictMax = 4095;
    std::string w;
    std::vector<uint16_t> codes;
    for (uint8_t c : data) {
        std::string wc = w + static_cast<char>(c);
        if (dict.count(wc)) {
            w = wc;
        } else {
            codes.push_back(dict[w]);
            if (dictSize < dictMax) {
                dict[wc] = dictSize++;
            }
            w = std::string(1, static_cast<char>(c));
        }
    }
    if (!w.empty()) codes.push_back(dict[w]);
    return codes;
}

std::vector<uint8_t> LZW::decodeChannel(const std::vector<uint16_t> &codes) {
    if (codes.empty()) return {};
    std::vector<std::string> dict(4096);
    for (int i = 0; i < 256; ++i) dict[i] = std::string(1, static_cast<char>(i));
    uint16_t dictSize = 256;
    std::string w(1, static_cast<char>(codes[0]));
    std::vector<uint8_t> out(w.begin(), w.end());
    for (size_t i = 1; i < codes.size(); ++i) {
        uint16_t k = codes[i];
        std::string entry;
        if (k < dictSize) {
            entry = dict[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        } else {
            throw std::runtime_error("Bad LZW code");
        }
        out.insert(out.end(), entry.begin(), entry.end());
        if (dictSize < 4096) {
            dict[dictSize++] = w + entry[0];
        }
        w = entry;
    }
    return out;
}

void LZW::compress(const cv::Mat &img, const std::string &outputPath) {
    ImageData data = ImageIO::fromMat(img);
    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file");
    ofs.write("LZW ", 4);
    ofs.write(reinterpret_cast<const char*>(&data.width), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char*>(&data.height), sizeof(uint32_t));
    ofs.put(static_cast<char>(data.channels));
    ofs.put(0); ofs.put(0); ofs.put(0);
    for (const auto &ch : data.channelData) {
        auto codes = encodeChannel(ch);
        uint32_t count = static_cast<uint32_t>(codes.size());
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
        for (uint16_t code : codes) {
            ofs.write(reinterpret_cast<const char*>(&code), sizeof(uint16_t));
        }
    }
}

cv::Mat LZW::decompress(const std::string &inputPath) {
    std::ifstream ifs(inputPath, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file");
    char magic[4]; ifs.read(magic, 4);
    if (std::string(magic,4) != "LZW ") throw std::runtime_error("Invalid magic for LZW");
    ImageData data;
    ifs.read(reinterpret_cast<char*>(&data.width), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&data.height), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&data.channels), 1);
    char pad[3]; ifs.read(pad,3);
    data.channelData.resize(data.channels);
    for (auto &ch : data.channelData) {
        uint32_t count = 0; ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
        std::vector<uint16_t> codes(count);
        for (uint32_t i = 0; i < count; ++i) {
            ifs.read(reinterpret_cast<char*>(&codes[i]), sizeof(uint16_t));
        }
        ch = decodeChannel(codes);
    }
    return ImageIO::toMat(data);
}
