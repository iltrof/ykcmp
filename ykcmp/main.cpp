#include "cxxopts.hpp"
#include "util.h"
#include "ykcmp.h"

#include <iostream>
#include <fstream>

bool hasValidHeader(std::ifstream& file);
size_t readArchiveSize(std::ifstream& file);
std::vector<char> fileToVector(std::ifstream& file, size_t size);

void compress(const std::string& inputName, const std::string& outputName, int level);
void decompress(const std::string& inputName, const std::string& outputName, size_t offset);

int main(int argc, char** argv) {
    std::string inputName;
    std::string outputName;

    bool quiet = false;
    bool showHelp = false;
    bool showLevels = false;

    bool compressMode = false;
    int compressionLevel;
    size_t archiveOffset;

    cxxopts::Options options("ykcmp", "(De)compressor for the YKCMP archive format");
    options.positional_help("INPUT [OUTPUT]");
    options.add_options()
        ("c,compress", "Compress instead of decompressing",     cxxopts::value<bool>       (compressMode))
        ("i,input",    "Input file",                            cxxopts::value<std::string>(inputName   ), "FILE")
        ("o,output",   "Output file",                           cxxopts::value<std::string>(outputName  ), "FILE")
        ("q,quiet",    "Do not write anything to console",      cxxopts::value<bool>       (quiet       ))
        ("help",       "Print help",                            cxxopts::value<bool>       (showHelp    ))
        ("levels",     "Print available compression levels",    cxxopts::value<bool>       (showLevels  ))
        ("rest",       "Positional arguments",                  cxxopts::value<std::vector<std::string>>());
    options.add_options("Decompression")
        ("a,at",       "Offset of the archive inside the file", cxxopts::value<size_t>(archiveOffset)->default_value("0"), "N");
    options.add_options("Compression")
        ("l,level",    "Level of compression (see --levels)",   cxxopts::value<int>(compressionLevel)->default_value("1"), "N");
    options.parse_positional({"input", "output", "rest"});

    auto opts = options.parse(argc, argv);

    if (showLevels || compressionLevel < 0 || compressionLevel > 2) {
        if (compressionLevel < 0 || compressionLevel > 2) {
            std::cout << "Invalid compression level\n\n";
        }

        std::cout << "Compression levels:\n"
            "  0: No compression (just produces valid YKCMP archives; fastest)\n"
            "  1: Decent (better than \"best\" for files with little repetition; moderately fast)\n"
            "  2: Best (slow)\n\n"
            "1 is recommended: it's way faster than 2, and the difference in size is negligible\n\n";
        return 0;
    } else if (showHelp || inputName == "") {
        std::cout << options.help({"", "Decompression", "Compression"}) << std::endl;
        return 0;
    }

    if (quiet) {
        setLogging(false);
    }

    if (outputName == "") {
        outputName = inputName + (compressMode ? ".ykcmp" : ".dec");
    }

    if (compressMode) {
        compress(inputName, outputName, compressionLevel);
    } else {
        decompress(inputName, outputName, archiveOffset);
    }

    return 0;
}

void compress(const std::string& inputName, const std::string& outputName, int level) {
    std::ifstream infile(inputName, std::ios::binary | std::ios::ate);
    if(infile.fail()) {
        std::cout << "Could not open input file: " << inputName << "\n";
        return;
    }

    size_t size = static_cast<size_t>(infile.tellg());
    infile.seekg(0);

    std::vector<char> input = fileToVector(infile, size);

    log() << "Compressing " + inputName + "...\n";
    log() << ".........|.........|.........|.........|.........|.........|.........|.........|.........|.........|\n";

    std::vector<char> output = yk::compress(input, level);

    std::ofstream outfile(outputName, std::ios::binary);
    if(outfile.fail()) {
        std::cout << "Could not open output file: " << outputName << "\n";
        return;
    }
    outfile.write(&output[0], output.size());

    log() << "Written 0x" << std::hex << output.size() << std::dec << " (" << output.size() << ") bytes to " << outputName << "\n";
    log() << "(" << static_cast<float>(output.size()) / input.size() * 100.f << "% of original file)\n\n";
}

void decompress(const std::string& inputName, const std::string& outputName, size_t offset) {
    std::ifstream infile(inputName, std::ios::binary);
    if(infile.fail()) {
        std::cout << "Could not open file: " << inputName << "\n";
        return;
    }
    infile.ignore(offset);

    if (!hasValidHeader(infile)) {
        std::cout << "Invalid archive format!\n"
            "Expected to see \"YKCMP_V1\" at the start of the file/given offset\n";
        return;
    }

    std::vector<char> input = fileToVector(infile, readArchiveSize(infile));

    log() << "Extracting " << inputName << "...\n";
    log() << ".........|.........|.........|.........|.........|.........|.........|.........|.........|.........|\n";

    std::vector<char> output = yk::decompress(input);

    std::ofstream outfile(outputName, std::ios::binary);
    if(outfile.fail()) {
        std::cout << "Could not open output file: " << outputName << "\n";
        return;
    }
    outfile.write(&output[0], output.size());

    log() << "Written 0x" << std::hex << output.size() << std::dec << " (" << output.size() << ") bytes to " << outputName << "\n\n";
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
