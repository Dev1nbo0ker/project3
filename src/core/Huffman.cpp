#include "Huffman.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <chrono>

namespace {
struct NodeCmp {
    bool operator()(HuffmanNode *a, HuffmanNode *b) const { return a->freq > b->freq; }
};

void freeTree(HuffmanNode *node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

void buildCodes(HuffmanNode *node, std::vector<bool> &path, std::array<std::vector<bool>,256> &table) {
    if (!node) return;
    if (node->value >= 0) {
        if (path.empty()) { // single symbol edge case
            path.push_back(false);
        }
        table[static_cast<uint8_t>(node->value)] = path;
        return;
    }
    path.push_back(false);
    buildCodes(node->left, path, table);
    path.back() = true;
    buildCodes(node->right, path, table);
    path.pop_back();
}
}

std::vector<uint8_t> Huffman::compressChannel(const std::vector<uint8_t> &data, uint64_t &validBits, std::array<uint64_t,256> &freqOut) {
    freqOut.fill(0);
    for (uint8_t v : data) freqOut[v]++;

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeCmp> pq;
    for (int i = 0; i < 256; ++i) {
        if (freqOut[i] > 0) {
            pq.push(new HuffmanNode{i, freqOut[i], nullptr, nullptr});
        }
    }
    if (pq.empty()) throw std::runtime_error("Empty data for Huffman compression");
    if (pq.size() == 1) {
        // Duplicate single node to avoid zero-length codes
        HuffmanNode *single = pq.top(); pq.pop();
        HuffmanNode *dummy = new HuffmanNode{-1, single->freq, single, new HuffmanNode{single->value, single->freq, nullptr, nullptr}};
        pq.push(dummy);
    }
    while (pq.size() > 1) {
        HuffmanNode *a = pq.top(); pq.pop();
        HuffmanNode *b = pq.top(); pq.pop();
        HuffmanNode *parent = new HuffmanNode{-1, a->freq + b->freq, a, b};
        pq.push(parent);
    }
    HuffmanNode *root = pq.top();
    std::array<std::vector<bool>,256> codeTable;
    std::vector<bool> path;
    buildCodes(root, path, codeTable);

    std::stringstream ss;
    ss.seekp(0);
    BitWriter writer(ss);
    for (uint8_t v : data) {
        const auto &bits = codeTable[v];
        for (bool b : bits) writer.writeBit(b ? 1 : 0);
    }
    writer.flush();
    validBits = writer.totalBitsWritten();
    std::string str = ss.str();
    std::vector<uint8_t> out(str.begin(), str.end());
    freeTree(root);
    return out;
}

static HuffmanNode* buildTreeFromFreq(const std::array<uint64_t,256> &freq) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeCmp> pq;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) pq.push(new HuffmanNode{i, freq[i], nullptr, nullptr});
    }
    if (pq.empty()) throw std::runtime_error("Invalid frequency table");
    if (pq.size() == 1) {
        HuffmanNode *single = pq.top(); pq.pop();
        HuffmanNode *dummy = new HuffmanNode{-1, single->freq, single, new HuffmanNode{single->value, single->freq, nullptr, nullptr}};
        pq.push(dummy);
    }
    while (pq.size() > 1) {
        HuffmanNode *a = pq.top(); pq.pop();
        HuffmanNode *b = pq.top(); pq.pop();
        pq.push(new HuffmanNode{-1, a->freq + b->freq, a, b});
    }
    return pq.top();
}

std::vector<uint8_t> Huffman::decompressChannel(const std::vector<uint8_t> &encoded, uint64_t validBits, const std::array<uint64_t,256> &freq) {
    HuffmanNode *root = buildTreeFromFreq(freq);
    std::stringstream ss;
    ss.write(reinterpret_cast<const char*>(encoded.data()), encoded.size());
    BitReader reader(ss);
    std::vector<uint8_t> output;
    HuffmanNode *cur = root;
    for (uint64_t i = 0; i < validBits; ++i) {
        int bit;
        if (!reader.readBit(bit)) break;
        cur = (bit == 0) ? cur->left : cur->right;
        if (!cur->left && !cur->right) {
            output.push_back(static_cast<uint8_t>(cur->value));
            cur = root;
        }
    }
    freeTree(root);
    return output;
}

void Huffman::compress(const cv::Mat &img, const std::string &outputPath) {
    ImageData data = ImageIO::fromMat(img);
    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file");
    ofs.write("HUFF", 4);
    ofs.write(reinterpret_cast<const char*>(&data.width), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char*>(&data.height), sizeof(uint32_t));
    ofs.put(static_cast<char>(data.channels));
    ofs.put(0); ofs.put(0); ofs.put(0);
    for (size_t c = 0; c < data.channelData.size(); ++c) {
        uint64_t validBits = 0;
        std::array<uint64_t,256> freq;
        auto encoded = compressChannel(data.channelData[c], validBits, freq);
        for (uint64_t f : freq) {
            ofs.write(reinterpret_cast<const char*>(&f), sizeof(uint64_t));
        }
        ofs.write(reinterpret_cast<const char*>(&validBits), sizeof(uint64_t));
        uint32_t sz = static_cast<uint32_t>(encoded.size());
        ofs.write(reinterpret_cast<const char*>(&sz), sizeof(uint32_t));
        ofs.write(reinterpret_cast<const char*>(encoded.data()), encoded.size());
    }
}

cv::Mat Huffman::decompress(const std::string &inputPath) {
    std::ifstream ifs(inputPath, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file");
    char magic[4];
    ifs.read(magic, 4);
    if (std::string(magic, 4) != "HUFF") throw std::runtime_error("Invalid magic for Huffman");
    ImageData data;
    ifs.read(reinterpret_cast<char*>(&data.width), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&data.height), sizeof(uint32_t));
    ifs.read(reinterpret_cast<char*>(&data.channels), 1);
    char pad[3]; ifs.read(pad, 3);
    data.channelData.resize(data.channels);
    for (size_t c = 0; c < data.channelData.size(); ++c) {
        std::array<uint64_t,256> freq;
        for (uint64_t &f : freq) {
            ifs.read(reinterpret_cast<char*>(&f), sizeof(uint64_t));
        }
        uint64_t validBits = 0; uint32_t sz = 0;
        ifs.read(reinterpret_cast<char*>(&validBits), sizeof(uint64_t));
        ifs.read(reinterpret_cast<char*>(&sz), sizeof(uint32_t));
        std::vector<uint8_t> encoded(sz);
        ifs.read(reinterpret_cast<char*>(encoded.data()), sz);
        data.channelData[c] = decompressChannel(encoded, validBits, freq);
    }
    return ImageIO::toMat(data);
}
