#include "ykcmp.h"
#include "util.h"

namespace yk {
    std::vector<char> compress(const std::vector<char>& data, const std::string& format, bool naive) {
        if(naive) {
            unsigned int chunks = data.size() / 0x7F;
            unsigned int rest = data.size() % 0x7F;

            size_t zsize = chunks * 0x80;
            if(rest != 0) zsize += rest + 1;

            std::vector<char> result = { 'Y','K','C','M','P','_','V','1','\x04','\0','\0','\0' };
            result.resize(zsize + 0x14);
            if(format == "v1") {
                writeU32(result, 0x0C, zsize + 0x14);
            } else if(format == "v1alt") {
                writeU32(result, 0x0C, zsize);
            }
            writeU32(result, 0x10, data.size());

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

            return result;
        } else {
            return {};
        }
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
