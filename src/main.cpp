#include <getopt.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <functional>

#include "BMPHeaders.h"
#include "colorModifiers.h"

void calculateBMPRow(const BMPDefaultInfoHeader& infoHeader, uint32_t& rowUnpadded, uint32_t& padding) {
    uint16_t bitCount = infoHeader.core.size == sizeof(BMPCoreHeader) ? infoHeader.core.bitCount : infoHeader.v1.bitCount;
    int bytesPerPixel = bitCount / 8;
    rowUnpadded = infoHeader.core.width * bytesPerPixel;
    padding = (4 - (rowUnpadded % 4)) % 4;
}

bool loadBMP(const std::string& filename, std::vector<char>& out, std::streamsize& size) {
    if(!std::filesystem::exists(filename)) {
        std::cerr << "File doesn't exist!\n";
        return false;
    }

    std::ifstream image(filename, std::ios::binary);
    if(!image) {
        std::cerr << "Can't open image!\n";
        return false;
    }

    image.seekg(0, std::ios::end);
    size = image.tellg();
    image.seekg(0, std::ios::beg);

    if((unsigned long)size < sizeof(BMPHeader) + sizeof(BMPCoreHeader) + 4) {
        std::cerr << "This file is invalid!\n";
        return false;
    }

    out.resize(size);

    if(!image.read(out.data(), size)) {
        std::cerr << "Failed to read the file.\n";
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    std::string filepath;
    std::string outputFile;
    std::function modifierFunction = rgbInvert;
    std::string defaultSuffix = "RGBInverted";
    bool printInfo = false;

    static struct option longOptions[] = {
        {"file", required_argument, nullptr, 'f'},
        {"output", optional_argument, nullptr, 'o'},
        {"info", no_argument, nullptr, 'i'},
        {"invert-rgb", no_argument, nullptr, 'r'},
        {"invert-hue", no_argument, nullptr, 'u'},
        {"invert-oklab", no_argument, nullptr, 'l'},
        {"flip-oklab-channels", no_argument, nullptr, 'c'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    int optIndex;
    while ((opt = getopt_long(argc, argv, "f:o:irulc", longOptions, &optIndex)) != -1) {
        switch (opt) {
            case 'f':
                filepath = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'i':
                printInfo = true;
                break;
            case 'r':
                modifierFunction = rgbInvert;
                break;
            case 'u':
                modifierFunction = hueInvert;
                defaultSuffix = "HSLHueInverted";
                break;
            case 'l':
                modifierFunction = oklabInvert;
                defaultSuffix = "OklabHueInverted";
                break;
            case 'c':
                modifierFunction = oklabFlip;
                defaultSuffix = "OklabABFlipped";
                break;
            case '?':
                break;
            default: ;
        }
    }

    std::streamsize size = 0;
    std::vector<char> buffer;

    if(!loadBMP(filepath, buffer, size)) return 1;
    if(outputFile.empty()) {
        outputFile = filepath.replace(filepath.size() - 4, 4, "");
        outputFile += defaultSuffix + ".bmp";
    } else if(!std::filesystem::is_directory(std::filesystem::path(outputFile).parent_path())) std::filesystem::create_directories(std::filesystem::path(outputFile).parent_path());

    auto* header = (BMPHeader*)buffer.data();
    auto* infoHeader = (BMPDefaultInfoHeader*)(buffer.data() + sizeof(BMPHeader));

    if(!checkBMPValidity(header, infoHeader, size)) {
        std::cerr << "Unsupported file\n";
        return 1;
    }

    if(printInfo) {
        printBMPHeader(header);
        printBMPInfoHeader(infoHeader);
        return 0;
    }

    uint32_t height = infoHeader->core.size == sizeof(BMPCoreHeader) ? infoHeader->core.height : infoHeader->v1.height;
    uint16_t bitCount = infoHeader->core.size == sizeof(BMPCoreHeader) ? infoHeader->core.bitCount : infoHeader->v1.bitCount;

    uint32_t byteCount = bitCount / 8;
    uint32_t rowLen = 0;
    uint32_t rowPaddingLen = 0;
    calculateBMPRow(*infoHeader, rowLen, rowPaddingLen);

    for(uint32_t currentY = 0; currentY < height; ++currentY) {
        unsigned char* bufCutout = (unsigned char*)buffer.data() + header->dataOffset + currentY * (rowLen + rowPaddingLen);
        for(uint32_t currentX = 0; currentX < infoHeader->core.width; currentX++) {
            uint8_t* pixelPtr = bufCutout + currentX * byteCount;
            // modifierFunction is responsible for writing the values to the pointers
            modifierFunction(&pixelPtr[2], &pixelPtr[1], &pixelPtr[0]);
        }
    }

    std::ofstream inverted(outputFile, std::ios::binary);
    inverted.write(buffer.data(), size);
    inverted.close();

    return 0;
}