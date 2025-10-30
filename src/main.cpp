#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <functional>

#include "BMPHeaders.h"
#include "args/cmdlineParser.h"
#include "args/colorspaceValueParser.h"
#include "color/colorModifiers.h"
#include "datatypes/Options.h"

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
    Options opt = parseCommandLineArgs(argc, argv);

    if (opt.colorSpace == ColorSpace::UNSET) {
        std::cerr << "There was an error parsing the command line.\n";
        return EXIT_FAILURE;
    }

    std::streamsize size = 0;
    std::vector<char> buffer;

    if(!loadBMP(opt.filePath, buffer, size)) return 1;

    auto* header = (BMPHeader*)buffer.data();
    auto* infoHeader = (BMPDefaultInfoHeader*)(buffer.data() + sizeof(BMPHeader));

    if(!checkBMPValidity(header, infoHeader, size)) {
        std::cerr << "Unsupported file\n";
        return 1;
    }

    if(opt.printInfo) {
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

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(opt, err);
    if (err) {
        std::cerr << "There was an error parsing the command line.\n";
        return EXIT_FAILURE;
    }

    for(uint32_t currentY = 0; currentY < height; ++currentY) {
        unsigned char* bufCutout = (unsigned char*)buffer.data() + header->dataOffset + currentY * (rowLen + rowPaddingLen);
        for(uint32_t currentX = 0; currentX < infoHeader->core.width; currentX++) {
            uint8_t* pixelPtr = bufCutout + currentX * byteCount;
            // modifierFunction is responsible for writing the values to the pointers
            opt.modifierFunc(&pixelPtr[2], &pixelPtr[1], &pixelPtr[0], modifiers, opt.allowOverflow);
        }
    }

    std::ofstream inverted(opt.outputPath, std::ios::binary);
    inverted.write(buffer.data(), size);
    inverted.close();

    return 0;
}