#include "..\ykcmp\util.cpp"
#include "..\ykcmp\ykcmp.cpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const int COMPRESSION_LEVELS = 3;

const std::vector<std::string> testNames = {
    "copy", "onebyte", "twobyte", "threebyte"
};

std::vector<char> fileToVector(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0);

    std::vector<char> result;
    result.resize(size);
    file.read(&result[0], size);
    return result;
}

bool runDecompressionTest(const std::string& name) {
    std::vector<char> input = fileToVector(std::string("tests/") + name + ".ykcmp");
    std::vector<char> output = yk::decompress(input);
    std::vector<char> expected = fileToVector(std::string("tests/") + name + ".dec");

    return output == expected;
}

bool runCompressionTest(const std::string& name, int level) {
    std::vector<char> input = fileToVector(std::string("tests/") + name + ".dec");
    std::vector<char> compressed = yk::compress(input, level);
    std::vector<char> decompressed = yk::decompress(compressed);

    return decompressed == input;
}

int main() {
    setLogging(false);

    for (auto& f : testNames) {
        bool success = runDecompressionTest(f);
        std::cout << (success ? "  Passed " : "! Failed ")
            << "decompression test \"" << f << "\"\n";
    }

    for (auto& f : testNames) {
        for (int level = 0; level < COMPRESSION_LEVELS; level++) {
            bool success = runCompressionTest(f, level);
            std::cout << (success ? "  Passed " : "! Failed ")
                << "compression test \"" << f << "\" at level " << level << "\n";
        }
    }

    return 0;
}
