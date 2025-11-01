#pragma once
#include "../datatypes/Options.h"
#include <filesystem>
#include <getopt.h>

inline Options parseCommandLineArgs(int argc, char** argv) {
    constexpr static option longOptions[] = {
        {"file", required_argument, nullptr, 'f'},
        {"output", required_argument, nullptr, 'o'},
        {"info", no_argument, nullptr, 'i'},
        {"rgb", required_argument, nullptr, 'r'},
        {"hsl", required_argument, nullptr, 'u'},
        {"oklab", required_argument, nullptr, 'l'},
        {"oklch", required_argument, nullptr, 'c'},
        {"verbose", no_argument, nullptr, 'v'},
        {nullptr, 0, nullptr, 0}
    };

    Options options;

    int opt;
    int optIndex;
    while ((opt = getopt_long(argc, argv, "f:o:r:u:l:i", longOptions, &optIndex)) != -1) {
        bool error = false;
        switch (opt) {
            // File parsing
            case 'f':
                error = !std::filesystem::is_regular_file(optarg);
                options.filePath = optarg;
                break;
            case 'o':
                error = std::filesystem::is_regular_file(optarg);
                options.outputPath = optarg;
                break;

            // Bool flags parsing
            case 'i':
                options.printInfo = true;
                break;
            case 'v':
                options.verbose = true;
                break;

            // Colorspace parsing
            case 'r':
            case 'u':
            case 'c':
            case 'l':
                options.colorSpace = (ColorSpace)opt;
                options.modifierArg = optarg;
                break;

            // Error handling
            case '?':
                break;
            default: ;
        }

        if (error) {
            std::cerr << "Error at option " << (char)opt << "\n";
            return {};
        }
    }
    return options;
}
