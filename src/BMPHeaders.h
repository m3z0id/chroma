#pragma once

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>

#pragma pack(push, 1)
typedef struct s_BMPChannelEndpoint {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} BMPChannelEndpoint;

typedef struct s_BMPHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
} BMPHeader;

typedef struct s_BMPCoreHeader {
    uint32_t size;
    uint16_t width;
    uint16_t height;
    uint16_t planes;
    uint16_t bitCount;
} BMPCoreHeader;

typedef struct s_BMPInfoHeader {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t xPixelsPerM;
    uint32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} BMPInfoHeader;

typedef struct s_BMPInfoHeaderV2 {
    BMPInfoHeader infoHeader;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
} BMPInfoHeaderV2;

typedef struct s_BMPInfoHeaderV3 {
    BMPInfoHeaderV2 v2InfoHeader;
    uint32_t alphaMask;
} BMPInfoHeaderV3;

typedef struct s_BMPInfoHeaderV4 {
    BMPInfoHeaderV3 v3InfoHeader;
    uint32_t colorSpaceType;
    BMPChannelEndpoint redEndpoint;
    BMPChannelEndpoint greenEndpoint;
    BMPChannelEndpoint blueEndpoint;
    uint32_t redGamma;
    uint32_t greenGamma;
    uint32_t blueGamma;
} BMPInfoHeaderV4;

typedef struct s_BMPInfoHeaderV5 {
    BMPInfoHeaderV4 v4InfoHeader;
    uint32_t intent;
    uint32_t profileData;
    uint32_t profileSize;
    uint32_t reserved;
} BMPInfoHeaderV5;
#pragma pack(pop)

union BMPDefaultInfoHeader {
    BMPCoreHeader core;
    BMPInfoHeader v1;
    BMPInfoHeaderV2 v2;
    BMPInfoHeaderV3 v3;
    BMPInfoHeaderV4 v4;
    BMPInfoHeaderV5 v5;
};

std::string fixed16_16ToString(uint32_t fx) {
    uint32_t intPart = fx >> 16;
    uint32_t fracPart = fx & 0xFFFF;

    uint64_t scale = 1;
    for (int i = 0; i < 6; ++i) scale *= 10ULL;

    uint64_t frac_decimal = (static_cast<uint64_t>(fracPart) * scale + 32768ULL) / 65536ULL;
    if (frac_decimal == scale) {
        frac_decimal = 0;
        ++intPart;
    }

    std::ostringstream ss;
    ss << intPart << '.' << std::setw(6) << std::setfill('0') << frac_decimal;
    return ss.str();
}

void printBMPHeader(const BMPHeader* header) {
    std::cout << "=== BMP Header ===\n";
    std::cout << "Signature      : 0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << header->signature << std::dec << "\n";
    std::cout << "File Size      : " << header->fileSize << " bytes\n";
    std::cout << "Data Offset    : " << header->dataOffset << " bytes\n";
}

void printBMPInfoHeader(const BMPDefaultInfoHeader* info) {
    std::cout << "=== BMP Info Header ===\n";
    std::cout << "Header Size        : " << info->core.size << " bytes\n";
    if(info->core.size == sizeof(BMPCoreHeader)) {
        std::cout << "Image Width        : " << info->core.width << " px\n";
        std::cout << "Image Height       : " << info->core.height << " px\n";
        std::cout << "Bits per pixel     : " << info->core.bitCount << "\n";
    } else {
        std::cout << "Image Width        : " << info->v1.width << " px\n";
        std::cout << "Image Height       : " << info->v1.height << " px\n";
        std::cout << "Color Planes       : " << info->v1.planes << "\n";
        std::cout << "Bits per Pixel     : " << info->v1.bitCount << "\n";
        std::cout << "Compression        : " << info->v1.compression << "\n";
        std::cout << "Image Size         : " << info->v1.imageSize << " bytes\n";
        std::cout << "X Pixels per Meter : " << info->v1.xPixelsPerM << "\n";
        std::cout << "Y Pixels per Meter : " << info->v1.yPixelsPerM << "\n";
        std::cout << "Colors Used        : " << info->v1.colorsUsed << "\n";
        std::cout << "Important Colors   : " << info->v1.colorsImportant << "\n";
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV2)) {
        std::cout << "Red Mask           : 0x" << std::hex << std::setw(8) << std::setfill('0') << info->v2.redMask << "\n";
        std::cout << "Green Mask         : 0x" << std::setw(8) << info->v2.greenMask << "\n";
        std::cout << "Blue Mask          : 0x" << std::setw(8) << info->v2.blueMask << std::dec << "\n";
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV3)) {
        std::cout << "Alpha Mask         : 0x" << std::hex << std::setw(8) << std::setfill('0') << info->v3.alphaMask << std::dec << "\n";
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV4)) {
        std::cout << "Red Endpoint       : " << fixed16_16ToString(info->v4.redEndpoint.x) << "; "
                  << fixed16_16ToString(info->v4.redEndpoint.y) << "; "
                  << fixed16_16ToString(info->v4.redEndpoint.z) << "\n";
        std::cout << "Red Gamma          : " << fixed16_16ToString(info->v4.redGamma) << "\n";
        std::cout << "Green Endpoint     : " << fixed16_16ToString(info->v4.greenEndpoint.x) << "; "
                  << fixed16_16ToString(info->v4.greenEndpoint.y) << "; "
                  << fixed16_16ToString(info->v4.greenEndpoint.z) << "\n";
        std::cout << "Green Gamma        : " << fixed16_16ToString(info->v4.greenGamma) << "\n";
        std::cout << "Blue Endpoint      : " << fixed16_16ToString(info->v4.blueEndpoint.x) << "; "
                  << fixed16_16ToString(info->v4.blueEndpoint.y) << "; "
                  << fixed16_16ToString(info->v4.blueEndpoint.z) << "\n";
        std::cout << "Blue Gamma         : " << fixed16_16ToString(info->v4.blueGamma) << "\n";
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV5)) {
        std::unordered_map<uint32_t, std::string> intents = {
            {0x00000001, "Maintaining Saturation"},
            {0x00000002, "Maintaining Colorimetric Match"},
            {0x00000004, "Maintaining Contrast"},
            {0x00000008, "Maintaining White Point"},
        };

        std::cout << "Rendering Intent   : " << intents[info->v5.intent] << " (" << info->v5.intent << ")\n";
        std::cout << "ICC Profile Offset : " << info->v5.profileData << "\n";
        std::cout << "ICC Profile Size   : " << info->v5.profileSize << "\n";
    }
}

bool checkBMPValidity(const BMPHeader* header, const BMPDefaultInfoHeader* infoHeader, const std::size_t& fileSize) {
    std::array<uint32_t, 6> validSizes = {sizeof(BMPCoreHeader), sizeof(BMPInfoHeader), sizeof(BMPInfoHeaderV2), sizeof(BMPInfoHeaderV3), sizeof(BMPInfoHeaderV4), sizeof(BMPInfoHeaderV5)};
    std::array<uint32_t, 6> validDataOffsets = {sizeof(BMPCoreHeader) + sizeof(BMPHeader), sizeof(BMPInfoHeader) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV2) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV3) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV4) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV5) + sizeof(BMPHeader)};
    bool val = header->fileSize == fileSize && std::ranges::find(validSizes, infoHeader->core.size) != validSizes.end() && header->reserved == 0 && std::ranges::find(validDataOffsets, header->dataOffset) != validDataOffsets.end() && header->dataOffset == infoHeader->core.size + sizeof(BMPHeader);

    if(infoHeader->core.size <= sizeof(BMPCoreHeader))
        val = val && infoHeader->core.planes == 1;

    if(infoHeader->core.size <= sizeof(BMPInfoHeaderV4) && infoHeader->core.size >= sizeof(BMPInfoHeader))
        val = val && infoHeader->v1.planes == 1 && (infoHeader->v1.bitCount == 24 || infoHeader->v1.bitCount == 32) && infoHeader->v1.compression == 0;
    
    if(infoHeader->core.size >= sizeof(BMPInfoHeaderV5))
        val = val && infoHeader->v5.reserved == 0;

    return val;
}