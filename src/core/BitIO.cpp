#include "BitIO.h"

BitWriter::BitWriter(std::ostream &os) : out(os), buffer(0), bitCount(0), totalBits(0) {}

void BitWriter::writeBit(int bit) {
    buffer = static_cast<uint8_t>((buffer << 1) | (bit & 1));
    bitCount++;
    totalBits++;
    if (bitCount == 8) {
        out.put(static_cast<char>(buffer));
        bitCount = 0;
        buffer = 0;
    }
}

void BitWriter::writeBits(uint32_t bits, int count) {
    for (int i = count - 1; i >= 0; --i) {
        writeBit((bits >> i) & 1);
    }
}

void BitWriter::flush() {
    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        out.put(static_cast<char>(buffer));
        buffer = 0;
        bitCount = 0;
    }
}

BitReader::BitReader(std::istream &is) : in(is), buffer(0), bitPos(8) {}

bool BitReader::readBit(int &bit) {
    if (bitPos == 8) {
        int val = in.get();
        if (val == EOF) {
            return false;
        }
        buffer = static_cast<uint8_t>(val);
        bitPos = 0;
    }
    bit = (buffer >> (7 - bitPos)) & 1;
    ++bitPos;
    return true;
}
