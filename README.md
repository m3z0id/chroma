# Chroma

Chroma is a Linux-only (for now) blazingly fast command-line tool for manipulating BMP images. It allows you to add filters in different color spaces and save the result to a new file.

## Features

*   Load and save BMP images
*   Modify the image using different filters in different color spaces
*   Print BMP information

## Available color spaces

*   RGB
*   HSL
*   OKLAB
*   OKLCh

## Usage

```bash
chroma -f <input file> [options]
```

### Options

*   `--<colorspace> <modifications>`: The modifications to apply to the image.
*   `--output <output file>`: The output file path.
*   `--info`: Print BMP information.

Modifications are in the format `<channel>:[+|-|~]<value|channel>` and different channel modifications are separated by `;`.
> Example of inverting RGB: `chroma -f input.bmp -o output.bmp --rgb r:~r;g:~g;b:~b`

## Building

To build Chroma, you will need a C++ compiler and CMake.

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
