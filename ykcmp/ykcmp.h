#pragma once
#include <vector>

namespace yk {
    std::vector<char> compress(const std::vector<char>& data, const std::string& format, bool naive = false);
    std::vector<char> decompress(const std::vector<char>& data);
}
