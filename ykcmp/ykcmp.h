#pragma once
#include <vector>

namespace yk {
    std::vector<char> compress(const std::vector<char>& data, const std::string& version);
    std::vector<char> decompress(const std::vector<char>& data);
}
