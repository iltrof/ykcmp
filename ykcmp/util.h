#pragma once
#include <vector>

unsigned int readU32(const std::vector<char>& vec, size_t at);
void writeU32(std::vector<char>& vec, size_t at, unsigned int value);