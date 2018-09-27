#include "..\ykcmp\util.cpp"
#include "..\ykcmp\ykcmp.cpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::vector<std::string> testNames = {
    "copy", "onebyte", "twobyte", "threebyte"
};

std::vector<char> fileToVector(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
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

bool runCompressionTest(const std::string& name, bool naive) {
    std::vector<char> input = fileToVector(std::string("tests/") + name + ".dec");
    std::vector<char> compressed = yk::compress(input, naive);
    std::vector<char> decompressed = yk::decompress(compressed);

    return decompressed == input;
}

int main() {
    for(auto& f : testNames) {
        bool success = runDecompressionTest(f);
        if(success) {
            std::cout << "  Passed decompression test \"" << f << "\"\n";
        } else {
            std::cout << "! Failed decompression test \"" << f << "\"\n";
        }
    }
    for(auto& f : testNames) {
        bool success = runCompressionTest(f, true);
        if(success) {
            std::cout << "  Passed naive compression test \"" << f << "\"\n";
        } else {
            std::cout << "! Failed naive compression test \"" << f << "\"\n";
        }
    }
    for(auto& f : testNames) {
        bool success = runCompressionTest(f, false);
        if(success) {
            std::cout << "  Passed normal compression test \"" << f << "\"\n";
        } else {
            std::cout << "! Failed normal compression test \"" << f << "\"\n";
        }
    }

    return 0;
}
