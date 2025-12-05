#pragma once
#include <cstdint>
#include <iostream>

// Simple bit writer/reader utilities.
// 位级IO辅助类，保持MSB-first一致性，便于Huffman编码。
class BitWriter {
public:
    explicit BitWriter(std::ostream &os);
    void writeBit(int bit);
    void writeBits(uint32_t bits, int count);
    void flush();
    uint64_t totalBitsWritten() const { return totalBits; }
private:
    std::ostream &out;
    uint8_t buffer;
    int bitCount;
    uint64_t totalBits;
};

class BitReader {
public:
    explicit BitReader(std::istream &is);
    bool readBit(int &bit);
private:
    std::istream &in;
    uint8_t buffer;
    int bitPos;
};
