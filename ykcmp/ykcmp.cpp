#include "ykcmp.h"
#include "util.h"

#include <iostream>

namespace yk {
    namespace {
        void countPercents(unsigned int& percentage, unsigned int part, unsigned int whole) {
            unsigned int newPerc = static_cast<unsigned int>(100.f * part / whole);
            if (newPerc > 100) {
                newPerc = 100;
            }
            for (; percentage < newPerc; percentage++) {
                log() << "#";
            }
        }

        void naiveCompress(const std::vector<char>& data, std::vector<char>& result, unsigned int chunks, unsigned int rest) {
            unsigned int percentage = 0;

            auto dIter = data.begin();
            auto rIter = result.begin() + 0x14;
            for (unsigned int i = 0; i < chunks; i++) {
                *(rIter++) = 0x7F;
                std::copy(dIter, dIter + 0x7F, rIter);
                dIter += 0x7F; rIter += 0x7F;

                countPercents(percentage, rIter - result.begin() - 0x14, result.size() - 0x14);
            }
            if (rest != 0) {
                *(rIter++) = static_cast<char>(rest);
                std::copy(dIter, dIter + rest, rIter);
            }

            countPercents(percentage, 100, 100);
            result.resize(rIter - result.begin());
        }

        void normalCompress(const std::vector<char>& data, std::vector<char>& result, int level) {
            unsigned int percentage = 0;

            auto dIter = data.begin();
            auto rIter = result.begin() + 0x14;
            auto copyHead = result.end();

            while (dIter < data.end()) {
                countPercents(percentage, dIter - data.begin(), data.size());

                const int maxSz[] = { 0x3, 0x1F, 0x1FF };
                int maxOffset[] = { 0x10, 0x100, 0x202 };
                if (level == 2) {
                    maxOffset[2] = 0x1000;
                }

                int sz = 0, offset = 0, saved = 0;
                for (int i = 3; i > 0; i--) {
                    for (auto start = dIter - maxOffset[i - 1]; start < dIter - saved - i; start++) {
                        if (start < data.begin()) {
                            start = data.begin();
                        }
                        if (*start != *dIter) {
                            continue;
                        }

                        int s = -i;
                        auto leftIter = start, rightIter = dIter;
                        while (rightIter < data.end() && *leftIter == *rightIter && s < maxSz[i - 1] && leftIter < dIter) {
                            leftIter++;
                            rightIter++;
                            s++;
                        }

                        if (s > saved) {
                            sz = s + i;
                            offset = dIter - start;
                            saved = s;
                        }
                    }
                }

                if (saved <= 0) {
                    if (copyHead == result.end() || *copyHead == 0x7F) {
                        copyHead = rIter++;
                        *copyHead = 0;
                    }

                    (*copyHead)++;
                    *(rIter++) = *(dIter++);
                    continue;
                }

                copyHead = result.end();

                if (sz <= 0x4 && offset <= 0x10) {
                    *(rIter++) = static_cast<char>((sz << 4) + 0x70 + (offset - 1));
                } else if (sz <= 0x21 && offset <= 0x100) {
                    *(rIter++) = static_cast<char>(sz + 0xC0 - 2);
                    *(rIter++) = static_cast<char>(offset - 1);
                } else {
                    int tmp = sz + 0xE00 - 3;
                    *(rIter++) = static_cast<char>(tmp >> 4);
                    *(rIter++) = static_cast<char>(((tmp & 0x0F) << 4) + ((offset - 1) >> 8));
                    *(rIter++) = static_cast<char>((offset - 1) & 0xFF);
                }

                dIter += sz;
            }

            countPercents(percentage, 100, 100);
            result.resize(rIter - result.begin());
        }
    }

    std::vector<char> compress(const std::vector<char>& data, int level) {
        unsigned int chunks = data.size() / 0x7F;
        unsigned int rest = data.size() % 0x7F;

        size_t maxZSize = chunks * 0x80;
        if (rest != 0) maxZSize += rest + 1;
        std::vector<char> result = { 'Y','K','C','M','P','_','V','1','\x04','\0','\0','\0' };
        result.resize(maxZSize + 0x14);
        writeU32(result, 0x10, data.size());

        if (level == 0) {
            naiveCompress(data, result, chunks, rest);
        } else {
            normalCompress(data, result, level);
        }

        writeU32(result, 0x0C, result.size());
        log() << '\n';
        return result;
    }

    std::vector<char> decompress(const std::vector<char>& data) {
        size_t finalSize = readU32(data, 0x10);
        std::vector<char> result;
        result.resize(finalSize);

        auto rIter = result.begin();
        auto dIter = data.begin() + 0x14;
        unsigned int percentage = 0;
        for (; dIter < data.end() && rIter < result.end(); ) {
            unsigned char tmp = *(dIter++);

            if (tmp < 0x80) {
                std::copy(dIter, dIter + tmp, rIter);
                dIter += tmp; rIter += tmp;
            } else {
                size_t sz, offset;
                if (tmp < 0xC0) {
                    sz = (tmp >> 4) - 0x8 + 1;
                    offset = (tmp & 0x0F) + 1;
                } else if (tmp < 0xE0) {
                    sz = tmp - 0xC0 + 2;
                    offset = static_cast<unsigned char>(*(dIter++)) + 1;
                } else {
                    unsigned char tmp2 = *(dIter++);
                    unsigned char tmp3 = *(dIter++);
                    sz = (tmp << 4) + (tmp2 >> 4) - 0xE00 + 3;
                    offset = ((tmp2 & 0x0F) << 8) + tmp3 + 1;
                }

                std::copy(rIter - offset, rIter - offset + sz, rIter);
                rIter += sz;
            }

            countPercents(percentage, rIter - result.begin(), finalSize);
        }

        countPercents(percentage, 100, 100);
        result.resize(rIter - result.begin());
        log() << '\n';
        return result;
    }
}
