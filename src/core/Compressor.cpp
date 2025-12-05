#include "Compressor.h"
#include "Huffman.h"
#include "RLE.h"
#include "LZW.h"
#include "DCTCodec.h"
#include <stdexcept>

static Algorithm parseAlgo(const std::string &name) {
    if (name == "huffman") return Algorithm::Huffman;
    if (name == "rle") return Algorithm::RLE;
    if (name == "lzw") return Algorithm::LZW;
    if (name == "dct") return Algorithm::DCT;
    throw std::runtime_error("Unknown algorithm: " + name);
}

void Compressor::compressImage(const std::string &algoName, const cv::Mat &img, const std::string &outputPath, int quality) {
    Algorithm algo = parseAlgo(algoName);
    switch (algo) {
        case Algorithm::Huffman:
            Huffman::compress(img, outputPath);
            break;
        case Algorithm::RLE:
            RLE::compress(img, outputPath);
            break;
        case Algorithm::LZW:
            LZW::compress(img, outputPath);
            break;
        case Algorithm::DCT:
            DCTCodec::compress(img, outputPath, quality);
            break;
    }
}
