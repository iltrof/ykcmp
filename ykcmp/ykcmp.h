#pragma once
#include <vector>

namespace yk {
    std::vector<char> compress(const std::vector<char>& data, int level = 1);
    std::vector<char> decompress(const std::vector<char>& data);
}
