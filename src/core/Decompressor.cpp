#include "Decompressor.h"
#include "Compressor.h"
#include "Huffman.h"
#include "RLE.h"
#include "LZW.h"
#include "DCTCodec.h"
#include <stdexcept>

// 解析算法名称，与压缩侧保持一致，确保解压时使用正确的编解码器。
static Algorithm parseAlgo(const std::string &name) {
    if (name == "huffman") return Algorithm::Huffman;
    if (name == "rle") return Algorithm::RLE;
    if (name == "lzw") return Algorithm::LZW;
    if (name == "dct") return Algorithm::DCT;
    throw std::runtime_error("Unknown algorithm: " + name);
}

cv::Mat Decompressor::decompressImage(const std::string &algoName, const std::string &inputPath) {
    Algorithm algo = parseAlgo(algoName);
    // 根据枚举调用对应解码逻辑，保持与压缩入口的对称性。
    switch (algo) {
        case Algorithm::Huffman:
            return Huffman::decompress(inputPath);
        case Algorithm::RLE:
            return RLE::decompress(inputPath);
        case Algorithm::LZW:
            return LZW::decompress(inputPath);
        case Algorithm::DCT:
            return DCTCodec::decompress(inputPath);
    }
    throw std::runtime_error("Unsupported algorithm");
}
