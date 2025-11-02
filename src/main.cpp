#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "BMPHeaders.h"
#include "args/cmdlineParser.h"
#include "args/colorspaceValueParser.h"
#include "color/colorModifiers.h"
#include "datatypes/Options.h"

void calculateBMPRow(const BMPDefaultInfoHeader &infoHeader, uint32_t &rowUnpadded, uint32_t &padding) {
    uint16_t bitCount = infoHeader.core.size == sizeof(BMPCoreHeader) ? infoHeader.core.bitCount : infoHeader.v1.bitCount;
    int bytesPerPixel = bitCount / 8;
    rowUnpadded = infoHeader.core.width * bytesPerPixel;
    padding = (4 - (rowUnpadded % 4)) % 4;
}

bool loadBMP(const std::string &filename, std::vector<char> &out, std::streamsize &size) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File doesn't exist!\n";
        return false;
    }

    std::ifstream image(filename, std::ios::binary);
    if (!image) {
        std::cerr << "Can't open image!\n";
        return false;
    }

    image.seekg(0, std::ios::end);
    size = image.tellg();
    image.seekg(0, std::ios::beg);

    if ((unsigned long)size < sizeof(BMPHeader) + sizeof(BMPCoreHeader) + 4) {
        std::cerr << "This file is invalid!\n";
        return false;
    }

    out.resize(size);

    if (!image.read(out.data(), size)) {
        std::cerr << "Failed to read the file.\n";
        return false;
    }
    return true;
}

void parseHeaders(const unsigned char* buffer, BMPHeader** header, BMPDefaultInfoHeader** infoHeader) {
    *header = (BMPHeader*)buffer;
    *infoHeader = (BMPDefaultInfoHeader*)(buffer + sizeof(BMPHeader));
}

void modify(unsigned char* buffer, const Options& opt, std::array<Modifier, 3>& mod, const BMPDefaultInfoHeader* const infoHeader, const BMPHeader* const header) {
    const uint32_t height = infoHeader->core.size == sizeof(BMPCoreHeader) ? infoHeader->core.height : infoHeader->v1.height;
    const uint16_t bitCount = infoHeader->core.size == sizeof(BMPCoreHeader) ? infoHeader->core.bitCount : infoHeader->v1.bitCount;

    const uint32_t byteCount = bitCount / 8;
    uint32_t rowLen = 0;
    uint32_t rowPaddingLen = 0;
    calculateBMPRow(*infoHeader, rowLen, rowPaddingLen);

    for (uint32_t currentY = 0; currentY < height; ++currentY) {
        unsigned char *bufCutout = buffer + header->dataOffset + currentY * (rowLen + rowPaddingLen);
        for (uint32_t currentX = 0; currentX < infoHeader->core.width; currentX++) {
            uint8_t *pixelPtr = bufCutout + currentX * byteCount;
            // modifierFunction is responsible for writing the values to the pointers
            opt.modifierFunc(&pixelPtr[2], &pixelPtr[1], &pixelPtr[0], mod);
        }
    }
}

#ifdef TESTS
#include <chrono>
#include <input.h>
#include <cstring>

void testRGBInvert(int& failedCount) {
    auto inputLocal = new unsigned char[INPUT_SIZE];
    std::memcpy(inputLocal, INPUT, INPUT_SIZE);

    Options options = {
        "",
        "",
        "r:~r;g:~g;b:~b",
        modifyRGB,
        ColorSpace::RGB,
        false,
        false
    };
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    BMPHeader* header = nullptr;
    BMPDefaultInfoHeader* infoHeader = nullptr;
    parseHeaders(inputLocal, &header, &infoHeader);

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(options, err);
    if (err) {
        std::cerr << "RGB Invert FAILED at parsing\n";
        delete[] inputLocal;
        failedCount += 1;
        return;
    }

    modify(inputLocal, options, modifiers, infoHeader, header);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "RGB Invert finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";
    delete[] inputLocal;
}

void testHSLInvert(int& failedCount) {
    auto inputLocal = new unsigned char[INPUT_SIZE];
    std::memcpy(inputLocal, INPUT, INPUT_SIZE);

    Options options = {
        "",
        "",
        "h:~h;s:~s;l:~l",
        modifyHSL,
        ColorSpace::HSL,
        false,
        false
    };

    BMPHeader* header = nullptr;
    BMPDefaultInfoHeader* infoHeader = nullptr;
    parseHeaders(inputLocal, &header, &infoHeader);

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(options, err);
    if (err) {
        std::cerr << "HSL Invert FAILED at parsing\n";
        delete[] inputLocal;
        failedCount += 1;
        return;
    }

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    modify(inputLocal, options, modifiers, infoHeader, header);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "HSL Invert finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";
    delete[] inputLocal;
}

void testOKLABInvert(int& failedCount) {
    auto inputLocal = new unsigned char[INPUT_SIZE];
    std::memcpy(inputLocal, INPUT, INPUT_SIZE);

    Options options = {
        "",
        "",
        "l:~l;a:~a;b:~b",
        modifyOKLAB,
        ColorSpace::OKLAB,
        false,
        false
    };

    BMPHeader* header = nullptr;
    BMPDefaultInfoHeader* infoHeader = nullptr;
    parseHeaders(inputLocal, &header, &infoHeader);

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(options, err);
    if (err) {
        std::cerr << "OKLAB Invert FAILED at parsing\n";
        delete[] inputLocal;
        failedCount += 1;
        return;
    }

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    modify(inputLocal, options, modifiers, infoHeader, header);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "OKLAB Invert finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";
    delete[] inputLocal;
}

void testOKLChInvert(int& failedCount) {
    auto inputLocal = new unsigned char[INPUT_SIZE];
    std::memcpy(inputLocal, INPUT, INPUT_SIZE);

    Options options = {
        "",
        "",
        "l:~l;c:~c;h:~h",
        modifyOKLCh,
        ColorSpace::OKLCH,
        false,
        false
    };

    BMPHeader* header = nullptr;
    BMPDefaultInfoHeader* infoHeader = nullptr;
    parseHeaders(inputLocal, &header, &infoHeader);

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(options, err);
    if (err) {
        std::cerr << "OKLCh Invert FAILED at parsing\n";
        delete[] inputLocal;
        failedCount += 1;
        return;
    }

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    modify(inputLocal, options, modifiers, infoHeader, header);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "OKLCh Invert finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";
    delete[] inputLocal;
}

void testOKHSLInvert(int& failedCount) {
    auto inputLocal = new unsigned char[INPUT_SIZE];
    std::memcpy(inputLocal, INPUT, INPUT_SIZE);

    Options options = {
        "",
        "",
        "h:~h;s:~s;l:~l",
        modifyOKHSL,
        ColorSpace::OKHSL,
        false,
        false
    };

    BMPHeader* header = nullptr;
    BMPDefaultInfoHeader* infoHeader = nullptr;
    parseHeaders(inputLocal, &header, &infoHeader);

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(options, err);
    if (err) {
        std::cerr << "OKHSL Invert FAILED at parsing\n";
        delete[] inputLocal;
        failedCount += 1;
        return;
    }

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    modify(inputLocal, options, modifiers, infoHeader, header);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "OKHSL Invert finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";
    delete[] inputLocal;
}

int runTests() {
    int failedCount = 0;

    testRGBInvert(failedCount);
    testHSLInvert(failedCount);
    testOKLABInvert(failedCount);
    testOKLChInvert(failedCount);
    testOKHSLInvert(failedCount);

    return failedCount;
}
#endif

int main(int argc, char **argv) {
    Options opt = parseCommandLineArgs(argc, argv);

#ifdef TESTS
    if (!opt.test) std::cout << "This build is intended for tests only, consider recompiling with proper flags.\n";
    else return runTests();
#endif

    if (opt.colorSpace == ColorSpace::UNSET) {
        std::cerr << "There was an error parsing the command line.\n";
        return EXIT_FAILURE;
    }

    std::streamsize size = 0;
    std::vector<char> buffer;

    if (!loadBMP(opt.filePath, buffer, size))
        return 1;

    BMPHeader* header = nullptr;
    BMPDefaultInfoHeader* infoHeader = nullptr;
    parseHeaders((unsigned char*)buffer.data(), &header, &infoHeader);

    if (!checkBMPValidity(header, infoHeader, size)) {
        std::cerr << "Unsupported file\n";
        return 1;
    }

    if (opt.printInfo) {
        printBMPHeader(header);
        printBMPInfoHeader(infoHeader);
        return 0;
    }

    bool err = false;
    std::array<Modifier, 3> modifiers = parseColorSpace(opt, err);
    if (err) {
        std::cerr << "There was an error parsing the command line.\n";
        return EXIT_FAILURE;
    }

    modify((unsigned char*)buffer.data(), opt, modifiers, infoHeader, header);

    std::ofstream inverted(opt.outputPath, std::ios::binary);
    inverted.write(buffer.data(), size);
    inverted.close();

    return 0;
}