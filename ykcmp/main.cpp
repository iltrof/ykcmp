#include "cxxopts.hpp"
#include "util.h"
#include "ykcmp.h"

#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    bool compressMode;

    cxxopts::Options options("ykcmp", "(De)compressor for the YKCMP archive format");
    options.positional_help("INPUT [OUTPUT]");
    options.add_options()
        ("c,compress", "Compress instead of decompressing", cxxopts::value<bool>(compressMode)->default_value("false"))
        ("i,input", "Input file", cxxopts::value<std::string>(), "FILE")
        ("o,output", "Output file", cxxopts::value<std::string>(), "FILE")
        ("help", "Print help")
        ("rest", "Positional arguments", cxxopts::value<std::vector<std::string>>());
    options.parse_positional({"input", "output", "rest"});
    options.add_options("Decompression")
        ("a,at", "Offset of the archive inside the file", cxxopts::value<size_t>()->default_value("0"), "N");
    options.add_options("Compression")
        ("f,format-version", "Archive format version. Supported: v1, v1alt", cxxopts::value<std::string>()->default_value("v1"), "VER")
        ("naive", "Use naive \"compression\" (does not compress anything, but produces valid YKCMP archives; really fast)");

    auto opts = options.parse(argc, argv);

    if(opts.count("help") || !opts.count("input")) {
        std::cout << options.help({"", "Decompression", "Compression"}) << std::endl;
        return 0;
    }

    std::string outputName;
    if(opts.count("output") == 0) {
        if(compressMode) {
            outputName = opts["input"].as<std::string>() + ".yk";
        } else {
            outputName = opts["input"].as<std::string>() + ".dec";
        }
    } else {
        outputName = opts["output"].as<std::string>();
    }

    if(!compressMode) {
        std::ifstream infile(opts["input"].as<std::string>(), std::ios::binary);
        infile.ignore(opts["at"].as<size_t>());

        auto startPos = infile.tellg();
        std::vector<char> header;
        header.resize(0x14);

        infile.read(&header[0], 0x14);
        if(std::string(header.begin(), header.begin() + 8) != "YKCMP_V1") {
            std::cout << "Unknown archive format!\n"
                "Expected to see \"YKCMP_V1\" in the beginning of the file\n";
            return 1;
        }
        infile.seekg(startPos);

        size_t zsize = readU32(header, 0x0C) + 0x14;
        std::vector<char> input;
        input.resize(zsize);
        infile.read(&input[0], zsize);
        input.resize(infile.gcount());
        infile.close();

        std::vector<char> output = yk::decompress(input);
        std::ofstream outfile(outputName, std::ios::binary);
        outfile.write(&output[0], output.size());
    } else {
        std::string format = opts["format-version"].as<std::string>();
        if(format != "v1" && format != "v1alt") {
            std::cout << "Unknown format version: " << format << "\nSupported are: v1, v1alt\n";
            return 1;
        }

        std::ifstream infile(opts["input"].as<std::string>(), std::ios::binary | std::ios::ate);
        size_t size = infile.tellg();
        infile.seekg(0);

        std::vector<char> input;
        input.resize(size);
        infile.read(&input[0], size);
        input.resize(infile.gcount());
        infile.close();

        std::vector<char> output = yk::compress(input, opts["format-version"].as<std::string>(), opts.count("naive") != 0);
        std::ofstream outfile(outputName, std::ios::binary);
        outfile.write(&output[0], output.size());
    }

    return 0;
}
