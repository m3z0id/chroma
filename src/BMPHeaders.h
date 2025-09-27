#pragma once

#include <cstdint>
#include <print>
#include <unordered_set>
#include <sstream>
#include <iomanip>

typedef struct __attribute((packed)) s_BMPChannelEndpoint {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} BMPChannelEndpoint;

typedef struct __attribute((packed)) s_BMPHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
} BMPHeader;

typedef struct __attribute((packed)) s_BMPCoreHeader {
    uint32_t size;
    uint16_t width;
    uint16_t height;
    uint16_t planes;
    uint16_t bitCount;
} BMPCoreHeader;

typedef struct __attribute((packed)) s_BMPInfoHeader {
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

typedef struct __attribute((packed)) s_BMPInfoHeaderV2 {
    BMPInfoHeader infoHeader;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
} BMPInfoHeaderV2;

typedef struct __attribute((packed)) s_BMPInfoHeaderV3 {
    BMPInfoHeaderV2 v2InfoHeader;
    uint32_t alphaMask;
} BMPInfoHeaderV3;

typedef struct __attribute((packed)) s_BMPInfoHeaderV4 {
    BMPInfoHeaderV3 v3InfoHeader;
    uint32_t colorSpaceType;
    BMPChannelEndpoint redEndpoint;
    BMPChannelEndpoint greenEndpoint;
    BMPChannelEndpoint blueEndpoint;
    uint32_t redGamma;
    uint32_t greenGamma;
    uint32_t blueGamma;
} BMPInfoHeaderV4;

typedef struct __attribute((packed)) s_BMPInfoHeaderV5 {
    BMPInfoHeaderV4 v4InfoHeader;
    uint32_t intent;
    uint32_t profileData;
    uint32_t profileSize;
    uint32_t reserved;
} BMPInfoHeaderV5;

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
    std::println("=== BMP Header ===");
    std::println("Signature      : 0x{:04X}", header->signature);
    std::println("File Size      : {} bytes", header->fileSize);
    std::println("Data Offset    : {} bytes", header->dataOffset);
}

void printBMPInfoHeader(const BMPDefaultInfoHeader* info) {
    std::unordered_map<uint32_t, std::string> headerSizeMap {
        {sizeof(BMPCoreHeader), "Core Header"},
        {sizeof(BMPInfoHeader), "Info Header v1"},
        {sizeof(BMPInfoHeaderV2), "Info Header v2"},
        {sizeof(BMPInfoHeaderV3), "Info Header v3"},
        {sizeof(BMPInfoHeaderV4), "Info Header v4"},
        {sizeof(BMPInfoHeaderV5), "Info Header v5"},
    };

    std::println("=== BMP Info Header ===");
    std::println("Header Size        : {} bytes ({})", info->core.size, headerSizeMap[info->core.size]);
    if(info->core.size == sizeof(BMPCoreHeader)) {
        std::println("Image Width        : {} px", info->core.width);
        std::println("Image Height       : {} px", info->core.height);
        std::println("Bits per pixel:    : {}", info->core.bitCount);
    } else {
        std::println("Image Width        : {} px", info->v1.width);
        std::println("Image Height       : {} px", info->v1.height);
        std::println("Color Planes       : {}", info->v1.planes);
        std::println("Bits per Pixel     : {}", info->v1.bitCount);
        std::println("Compression        : {}", info->v1.compression);
        std::println("Image Size         : {} bytes", info->v1.imageSize);
        std::println("X Pixels per Meter : {}", info->v1.xPixelsPerM);
        std::println("Y Pixels per Meter : {}", info->v1.yPixelsPerM);
        std::println("Colors Used        : {}", info->v1.colorsUsed);
        std::println("Important Colors   : {}", info->v1.colorsImportant);
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV2)) {
        std::println("Red Mask           : 0x{:08X}", info->v2.redMask);
        std::println("Green Mask         : 0x{:08X}", info->v2.greenMask);
        std::println("Blue Mask          : 0x{:08X}", info->v2.blueMask);
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV3)) {
        std::println("Alpha Mask         : 0x{:08X}", info->v3.alphaMask);
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV4)) {
        std::println("Red Endpoint       : {}; {}; {}", fixed16_16ToString(info->v4.redEndpoint.x), fixed16_16ToString(info->v4.redEndpoint.y), fixed16_16ToString(info->v4.redEndpoint.z));
        std::println("Red Gamma          : {}", fixed16_16ToString(info->v4.redGamma));
        std::println("Green Endpoint     : {}; {}; {}", fixed16_16ToString(info->v4.greenEndpoint.x), fixed16_16ToString(info->v4.greenEndpoint.y), fixed16_16ToString(info->v4.greenEndpoint.z));
        std::println("Green Gamma        : {}", fixed16_16ToString(info->v4.greenGamma));
        std::println("Blue Endpoint      : {}; {}; {}", fixed16_16ToString(info->v4.blueEndpoint.x), fixed16_16ToString(info->v4.blueEndpoint.y), fixed16_16ToString(info->v4.blueEndpoint.z));
        std::println("Blue Gamma         : {}", fixed16_16ToString(info->v4.blueGamma));
    }
    if(info->core.size >= sizeof(BMPInfoHeaderV5)) {
        std::unordered_map<uint32_t, std::string> intents = {
            {0x00000001, "Maintaining Saturation"},
            {0x00000002, "Maintaining Colorimetric Match"},
            {0x00000004, "Maintaining Contrast"},
            {0x00000008, "Maintaining White Point"},
        };

        std::println("Rendering Intent   : {} ({})", intents[info->v5.intent], info->v5.intent);
        std::println("ICC Profile Offset : {}", info->v5.profileData);
        std::println("ICC Profile Size   : {}", info->v5.profileSize);
    }
}

bool checkBMPValidity(const BMPHeader* header, const BMPDefaultInfoHeader* infoHeader, const std::size_t& fileSize) {
    std::unordered_set<uint32_t> validSizes = {sizeof(BMPCoreHeader), sizeof(BMPInfoHeader), sizeof(BMPInfoHeaderV2), sizeof(BMPInfoHeaderV3), sizeof(BMPInfoHeaderV4), sizeof(BMPInfoHeaderV5)};
    std::unordered_set<uint32_t> validDataOffsets = {sizeof(BMPCoreHeader) + sizeof(BMPHeader), sizeof(BMPInfoHeader) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV2) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV3) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV4) + sizeof(BMPHeader), sizeof(BMPInfoHeaderV5) + sizeof(BMPHeader)};
    bool val = header->fileSize == fileSize && validSizes.contains(infoHeader->core.size) && header->reserved == 0 && validDataOffsets.contains(header->dataOffset) && header->dataOffset == infoHeader->core.size + sizeof(BMPHeader);

    if(infoHeader->core.size <= sizeof(BMPCoreHeader))
        val = val && infoHeader->core.planes == 1;

    if(infoHeader->core.size <= sizeof(BMPInfoHeaderV4) && infoHeader->core.size >= sizeof(BMPInfoHeader))
        val = val && infoHeader->v1.planes == 1 && infoHeader->v1.bitCount == 24 && infoHeader->v1.compression == 0;
    
    if(infoHeader->core.size >= sizeof(BMPInfoHeaderV5))
        val = val && infoHeader->v5.reserved == 0;

    return val;
}