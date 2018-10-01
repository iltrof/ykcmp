#include "util.h"

unsigned int readU32(const std::vector<char>& vec, size_t at) {
    return (static_cast<unsigned char>(vec[at]))
         + (static_cast<unsigned char>(vec[at + 1]) << 8)
         + (static_cast<unsigned char>(vec[at + 2]) << 16)
         + (static_cast<unsigned char>(vec[at + 3]) << 24);
}

void writeU32(std::vector<char>& vec, size_t at, unsigned int value) {
    vec[at]     = static_cast<char>((value & 0x000000FF));
    vec[at + 1] = static_cast<char>((value & 0x0000FF00) >> 8);
    vec[at + 2] = static_cast<char>((value & 0x00FF0000) >> 16);
    vec[at + 3] = static_cast<char>((value & 0xFF000000) >> 24);
}
