#include "ykcmp.h"
#include "util.h"

namespace yk {
    std::vector<char> compress(const std::vector<char>& data, bool naive) {
        unsigned int chunks = data.size() / 0x7F;
        unsigned int rest = data.size() % 0x7F;

        size_t maxZSize = chunks * 0x80;
        if(rest != 0) maxZSize += rest + 1;
        std::vector<char> result = { 'Y','K','C','M','P','_','V','1','\x04','\0','\0','\0' };
        result.resize(maxZSize + 0x14);
        writeU32(result, 0x10, data.size());

        if(naive) {
            auto dIter = data.begin();
            auto rIter = result.begin() + 0x14;
            for(int i = 0; i < chunks; i++) {
                *(rIter++) = 0x7F;
                std::copy(dIter, dIter + 0x7F, rIter);
                dIter += 0x7F; rIter += 0x7F;
            }
            if(rest != 0) {
                *(rIter++) = rest;
                std::copy(dIter, dIter + rest, rIter);
            }

            writeU32(result, 0x0C, maxZSize + 0x14);
        } else {
            auto dIter = data.begin();
            auto rIter = result.begin() + 0x14;
            auto copyHead = result.end();

            while(dIter < data.end()) {
                int sz = 0, offset = 0, saved = 0;
                for(auto i = dIter - 0x1000; i < dIter; i++) {
                    if(i < data.begin()) {
                        i = data.begin();
                    }

                    int s = -3;
                    auto leftIter = i, rightIter = dIter;
                    while(rightIter < data.end() && *leftIter == *rightIter && leftIter < dIter && s < 0x1FF) {
                        leftIter++;
                        rightIter++;
                        s++;
                    }

                    if(s > saved) {
                        sz = s + 3;
                        offset = dIter - i;
                        saved = s;
                    }
                }
                for(auto i = dIter - 0x100; i < dIter; i++) {
                    if(i < data.begin()) {
                        i = data.begin();
                    }

                    int s = -2;
                    auto leftIter = i, rightIter = dIter;
                    while(rightIter < data.end() && *leftIter == *rightIter && leftIter < dIter && s < 0x1F) {
                        leftIter++;
                        rightIter++;
                        s++;
                    }

                    if(s > saved) {
                        sz = s + 2;
                        offset = dIter - i;
                        saved = s;
                    }
                }
                for(auto i = dIter - 0x10; i < dIter; i++) {
                    if(i < data.begin()) {
                        i = data.begin();
                    }

                    int s = -1;
                    auto leftIter = i, rightIter = dIter;
                    while(rightIter < data.end() && *leftIter == *rightIter && leftIter < dIter && s < 0x3) {
                        leftIter++;
                        rightIter++;
                        s++;
                    }

                    if(s > saved) {
                        sz = s + 1;
                        offset = dIter - i;
                        saved = s;
                    }
                }

                if(copyHead != result.end()) {
                    if(saved <= 0) {
                        if(*copyHead >= 0x7F) {
                            copyHead = rIter;
                            *(rIter++) = 0;
                        }
                        *(rIter++) = *(dIter++);
                        (*copyHead)++;
                        continue;
                    } else {
                        copyHead = result.end();
                    }
                }

                if(saved <= 0) {
                    copyHead = rIter;
                    *(rIter++) = 1;
                    *(rIter++) = *(dIter++);
                    continue;
                }

                if(sz <= 0x4 && offset <= 0x10) {
                    *(rIter++) = (sz << 4) + 0x70 + (offset - 1);
                } else if(sz <= 0x21 && offset <= 0x100) {
                    *(rIter++) = sz + 0xC0 - 2;
                    *(rIter++) = offset - 1;
                } else {
                    int tmp = sz + 0xE00 - 3;
                    *(rIter++) = (tmp >> 4);
                    *(rIter++) = ((tmp & 0x0F) << 4) + ((offset - 1) >> 8);
                    *(rIter++) = ((offset - 1) & 0xFF);
                }

                dIter += sz;
            }

            size_t zsize = rIter - result.begin();
            result.resize(zsize);
            writeU32(result, 0x0C, zsize);
        }

        return result;
    }

    std::vector<char> decompress(const std::vector<char>& data) {
        size_t finalSize = readU32(data, 0x10);
        std::vector<char> result;
        result.resize(finalSize);

        auto rIter = result.begin();
        auto dIter = data.begin() + 0x14;
        for(; dIter < data.end() && rIter < result.end(); ) {
            unsigned char tmp = *(dIter++);

            if(tmp < 0x80) {
                std::copy(dIter, dIter + tmp, rIter);
                dIter += tmp; rIter += tmp;
            } else {
                size_t sz, offset;
                if(tmp < 0xC0) {
                    sz = (tmp >> 4) - 0x8 + 1;
                    offset = (tmp & 0x0F) + 1;
                } else if(tmp < 0xE0) {
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
        }

        return result;
    }
}
