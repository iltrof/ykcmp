#include "cxxopts.hpp"
#include "util.h"
#include "ykcmp.h"

#include <fstream>
#include <iostream>

bool hasValidHeader(std::ifstream& file);
size_t readArchiveSize(std::ifstream& file);
std::vector<char> fileToVector(std::ifstream& file, size_t size);

void compress(const std::string& inputName, const std::string& outputName, int level);
void decompress(const std::string& inputName, const std::string& outputName, size_t offset);

int main(int argc, char** argv) {
    bool compressMode;
    int compressionLevel;

    cxxopts::Options options("ykcmp", "(De)compressor for the YKCMP archive format");
    options.positional_help("INPUT [OUTPUT]");
    options.add_options()
        ("c,compress", "Compress instead of decompressing", cxxopts::value<bool>(compressMode)->default_value("false"))
        ("i,input", "Input file", cxxopts::value<std::string>(), "FILE")
        ("o,output", "Output file", cxxopts::value<std::string>(), "FILE")
        ("help", "Print help")
        ("levels", "Print available compression levels")
        ("rest", "Positional arguments", cxxopts::value<std::vector<std::string>>());
    options.parse_positional({"input", "output", "rest"});
    options.add_options("Decompression")
        ("a,at", "Offset of the archive inside the file", cxxopts::value<size_t>()->default_value("0"), "N");
    options.add_options("Compression")
        ("l,level", "Level of compression (see --levels)", cxxopts::value<int>(compressionLevel)->default_value("1"), "N");

    auto opts = options.parse(argc, argv);

    if (opts.count("levels") != 0 || compressionLevel < 0 || compressionLevel > 2) {
        if (compressionLevel < 0 || compressionLevel > 2) {
            std::cout << "Invalid compression level\n\n";
        }

        std::cout << "Compression levels:\n"
            "  0: No compression (just produces valid YKCMP archives; fastest)\n"
            "  1: Decent (better than \"best\" for files with little repetition; moderately fast)\n"
            "  2: Best (slow)\n\n"
            "1 is recommended: it's way faster than 2, and the difference in size is negligible\n\n";
        return 0;
    } else if (opts.count("help") != 0 || opts.count("input") == 0) {
        std::cout << options.help({"", "Decompression", "Compression"}) << std::endl;
        return 0;
    }

    std::string outputName, inputName;
    inputName = opts["input"].as<std::string>();
    if (opts.count("output") == 0) {
        outputName = inputName + (compressMode ? ".ykcmp" : ".dec");
    } else {
        outputName = opts["output"].as<std::string>();
    }

    if (compressMode) {
        compress(inputName, outputName, compressionLevel);
    } else {
        decompress(inputName, outputName, opts["at"].as<size_t>());
    }

    return 0;
}

void compress(const std::string& inputName, const std::string& outputName, int level) {
    std::ifstream infile(inputName, std::ios::binary | std::ios::ate);
    size_t size = static_cast<size_t>(infile.tellg());
    infile.seekg(0);

    std::vector<char> input = fileToVector(infile, size);

    std::cout << "Compressing " << inputName << "...\n";
    std::cout << ".........|.........|.........|.........|.........|.........|.........|.........|.........|.........|\n";

    std::vector<char> output = yk::compress(input, level);

    std::ofstream outfile(outputName, std::ios::binary);
    outfile.write(&output[0], output.size());

    std::cout << "Written 0x" << std::hex << output.size() << std::dec << " (" << output.size() << ") bytes to " << outputName << "\n";
    std::cout << "(" << static_cast<float>(output.size()) / input.size() * 100.f << "% of original file)\n";
}

void decompress(const std::string& inputName, const std::string& outputName, size_t offset) {
    std::ifstream infile(inputName, std::ios::binary);
    infile.ignore(offset);

    if (!hasValidHeader(infile)) {
        std::cout << "Invalid archive format!\n"
            "Expected to see \"YKCMP_V1\" in the beginning of the file\n";
        return;
    }

    std::vector<char> input = fileToVector(infile, readArchiveSize(infile));

    std::cout << "Extracting " << inputName << "...\n";
    std::cout << ".........|.........|.........|.........|.........|.........|.........|.........|.........|.........|\n";

    std::vector<char> output = yk::decompress(input);

    std::ofstream outfile(outputName, std::ios::binary);
    outfile.write(&output[0], output.size());

    std::cout << "Written 0x" << std::hex << output.size() << std::dec << " (" << output.size() << ") bytes to " << outputName << "\n";
}

bool hasValidHeader(std::ifstream& file) {
    auto oldPos = file.tellg();
    char magic[8];
    file.read(magic, 8);
    file.seekg(oldPos);
    return file.gcount() == 8 && std::string(magic, magic + 8) == "YKCMP_V1";
}

size_t readArchiveSize(std::ifstream& file) {
    auto oldPos = file.tellg();
    file.ignore(0x0C);
    size_t zsize;
    file.read(reinterpret_cast<char*>(&zsize), 4);
    file.seekg(oldPos);
    return zsize;
}

std::vector<char> fileToVector(std::ifstream& file, size_t size) {
    std::vector<char> result;
    result.resize(size);
    file.read(&result[0], size);
    result.resize(static_cast<unsigned int>(file.gcount()));
    return result;
}
