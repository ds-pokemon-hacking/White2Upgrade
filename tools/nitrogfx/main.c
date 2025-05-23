// Copyright (c) 2015 YamaArashi

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "global.h"
#include "util.h"
#include "options.h"
#include "gfx.h"
#include "convert_png.h"
#include "jasc_pal.h"
#include "lz.h"
#include "rl.h"
#include "font.h"
#include "huff.h"

struct CommandHandler
{
    const char *inputFileExtension;
    const char *outputFileExtension;
    void(*function)(char *inputPath, char *outputPath, int argc, char **argv);
};

void ConvertGbaToPng(char *inputPath, char *outputPath, struct GbaToPngOptions *options)
{
    struct Image image;

    if (options->paletteFilePath != NULL)
    {
        ReadGbaPalette(options->paletteFilePath, &image.palette);
        image.hasPalette = true;
    }
    else
    {
        image.hasPalette = false;
    }

    ReadImage(inputPath, options->width, options->bitDepth, options->metatileWidth, options->metatileHeight, &image, !image.hasPalette);

    image.hasTransparency = options->hasTransparency;

    WritePng(outputPath, &image);

    FreeImage(&image);
}

void ConvertNtrToPng(char *inputPath, char *outputPath, struct NtrToPngOptions *options)
{
    // handle empty files if possible
    FILE *fp = fopen(inputPath, "rb");

    if (options->handleEmpty)
    {
        if (fp != NULL)
        {
            fseek(fp, 0, SEEK_END);
            uint32_t size = ftell(fp);
            rewind(fp);
            if (size == 0)
            {
                FILE *out = fopen(outputPath, "wb+");
                fclose(out);
                fclose(fp);
                return;
            }
        }
    }

    fclose(fp);

    struct Image image;

    if (options->paletteFilePath != NULL)
    {
        ReadNtrPalette(options->paletteFilePath, &image.palette, options->bitDepth, options->palIndex);
        image.hasPalette = true;
    }
    else
    {
        image.hasPalette = false;
    }

    uint32_t key = ReadNtrImage(inputPath, options->width, 0, options->metatileWidth, options->metatileHeight, &image, !image.hasPalette);

    if (key)
    {
        char string[strlen(outputPath) + 5];
        sprintf(string, "%s.key", outputPath);
        FILE *fp = fopen(string, "wb");
        if (fp == NULL)
            FATAL_ERROR("Failed to open key file for writing.\n");
        fwrite(&key, 4, 1, fp);
        fclose(fp);
    }

    image.hasTransparency = options->hasTransparency;

    WritePng(outputPath, &image);

    FreeImage(&image);
}

void ConvertPngToGba(char *inputPath, char *outputPath, struct PngToGbaOptions *options)
{
    struct Image image;

    image.bitDepth = options->bitDepth;

    ReadPng(inputPath, &image);

    WriteImage(outputPath, options->numTiles, options->bitDepth, options->metatileWidth, options->metatileHeight, &image, !image.hasPalette);

    FreeImage(&image);
}

void ConvertPngToNtr(char *inputPath, char *outputPath, struct PngToNtrOptions *options)
{
    // handle empty files if possible
    FILE *fp = fopen(inputPath, "rb");

    if (options->handleEmpty)
    {
        if (fp != NULL)
        {
            fseek(fp, 0, SEEK_END);
            uint32_t size = ftell(fp);
            rewind(fp);
            if (size == 0)
            {
                FILE *out = fopen(outputPath, "wb+");
                fclose(out);
                fclose(fp);
                return;
            }
        }
    }

    fclose(fp);

    struct Image image;

    image.bitDepth = options->bitDepth;

    ReadPng(inputPath, &image);

    uint32_t key = 0;
    if (options->scanMode)
    {
        char string[strlen(inputPath) + 5];
        sprintf(string, "%s.key", inputPath);
        FILE *fp2 = fopen(string, "rb");
        if (fp2 == NULL)
            FATAL_ERROR("Failed to open key file for reading.\n");
        size_t count = fread(&key, 4, 1, fp2);
        if (count != 1)
            FATAL_ERROR("Not a valid key file.\n");
        fclose(fp2);
    }

    WriteNtrImage(outputPath, options->numTiles, image.bitDepth, options->metatileWidth, options->metatileHeight, &image, !image.hasPalette, options->clobberSize, options->byteOrder, options->version101, options->sopc, options->scanMode, key, options->bitmap);

    FreeImage(&image);
}

void HandleGbaToPngCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    char *inputFileExtension = GetFileExtension(inputPath);
    struct GbaToPngOptions options;
    options.paletteFilePath = NULL;
    if (isdigit(inputFileExtension[0]))
        options.bitDepth = inputFileExtension[0] - '0';
    else
        options.bitDepth = 4;
    options.hasTransparency = false;
    options.width = 1;
    options.metatileWidth = 1;
    options.metatileHeight = 1;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-palette") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No palette file path following \"-palette\".\n");

            i++;

            options.paletteFilePath = argv[i];
        }
        else if (strcmp(option, "-object") == 0)
        {
            options.hasTransparency = true;
        }
        else if (strcmp(option, "-width") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No width following \"-width\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.width))
                FATAL_ERROR("Failed to parse width.\n");

            if (options.width < 1)
                FATAL_ERROR("Width must be positive.\n");
        }
        else if (strcmp(option, "-mwidth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile width value following \"-mwidth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileWidth))
                FATAL_ERROR("Failed to parse metatile width.\n");

            if (options.metatileWidth < 1)
                FATAL_ERROR("metatile width must be positive.\n");
        }
        else if (strcmp(option, "-mheight") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile height value following \"-mheight\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileHeight))
                FATAL_ERROR("Failed to parse metatile height.\n");

            if (options.metatileHeight < 1)
                FATAL_ERROR("metatile height must be positive.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    if (options.metatileWidth > options.width)
        options.width = options.metatileWidth;

    ConvertGbaToPng(inputPath, outputPath, &options);
}

void HandleNtrToPngCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    struct NtrToPngOptions options;
    options.paletteFilePath = NULL;
    options.hasTransparency = false;
    options.width = 0;
    options.metatileWidth = 1;
    options.metatileHeight = 1;
    options.palIndex = 1;
    options.scanFrontToBack = false;
    options.handleEmpty = false;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-palette") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No palette file path following \"-palette\".\n");

            i++;

            options.paletteFilePath = argv[i];
        }
        else if (strcmp(option, "-object") == 0)
        {
            options.hasTransparency = true;
        }
        else if (strcmp(option, "-palindex") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No palette index following \"-palindex\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.palIndex))
                FATAL_ERROR("Failed to parse palette index.\n");

            if (options.width < 1)
                FATAL_ERROR("Palette index must be positive.\n");
        }
        else if (strcmp(option, "-width") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No width following \"-width\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.width))
                FATAL_ERROR("Failed to parse width.\n");

            if (options.width < 1)
                FATAL_ERROR("Width must be positive.\n");
        }
        else if (strcmp(option, "-mwidth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile width value following \"-mwidth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileWidth))
                FATAL_ERROR("Failed to parse metatile width.\n");

            if (options.metatileWidth < 1)
                FATAL_ERROR("metatile width must be positive.\n");
        }
        else if (strcmp(option, "-mheight") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile height value following \"-mheight\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileHeight))
                FATAL_ERROR("Failed to parse metatile height.\n");

            if (options.metatileHeight < 1)
                FATAL_ERROR("metatile height must be positive.\n");
        }
        else if (strcmp(option, "-scanfronttoback") == 0)
        {
            options.scanFrontToBack = true;
        }
        else if (strcmp(option, "-handleempty") == 0)
        {
            options.handleEmpty = true;
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    if (options.width != 0 && options.metatileWidth > options.width)
        options.width = options.metatileWidth;

    ConvertNtrToPng(inputPath, outputPath, &options);
}

void HandlePngToGbaCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    char *outputFileExtension = GetFileExtension(outputPath);
    int bitDepth;
    if (strcmp(outputFileExtension, "nbfc") == 0)
        bitDepth = 4;
    else
        bitDepth = outputFileExtension[0] - '0';
    struct PngToGbaOptions options;
    options.numTiles = 0;
    options.bitDepth = bitDepth;
    options.metatileWidth = 1;
    options.metatileHeight = 1;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-num_tiles") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No number of tiles following \"-num_tiles\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.numTiles))
                FATAL_ERROR("Failed to parse number of tiles.\n");

            if (options.numTiles < 1)
                FATAL_ERROR("Number of tiles must be positive.\n");
        }
        else if (strcmp(option, "-mwidth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile width value following \"-mwidth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileWidth))
                FATAL_ERROR("Failed to parse metatile width.\n");

            if (options.metatileWidth < 1)
                FATAL_ERROR("metatile width must be positive.\n");
        }
        else if (strcmp(option, "-mheight") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile height value following \"-mheight\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileHeight))
                FATAL_ERROR("Failed to parse metatile height.\n");

            if (options.metatileHeight < 1)
                FATAL_ERROR("metatile height must be positive.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    ConvertPngToGba(inputPath, outputPath, &options);
}

void HandlePngToNtrCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    struct PngToNtrOptions options;
    options.numTiles = 0;
    options.bitDepth = 4;
    options.metatileWidth = 1;
    options.metatileHeight = 1;
    options.clobberSize = false;
    options.byteOrder = true;
    options.version101 = false;
    options.sopc = false;
    options.scanMode = 0;
    options.handleEmpty = false;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-num_tiles") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No number of tiles following \"-num_tiles\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.numTiles))
                FATAL_ERROR("Failed to parse number of tiles.\n");

            if (options.numTiles < 1)
                FATAL_ERROR("Number of tiles must be positive.\n");
        }
        else if (strcmp(option, "-mwidth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile width value following \"-mwidth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileWidth))
                FATAL_ERROR("Failed to parse metatile width.\n");

            if (options.metatileWidth < 1)
                FATAL_ERROR("metatile width must be positive.\n");
        }
        else if (strcmp(option, "-mheight") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No metatile height value following \"-mheight\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.metatileHeight))
                FATAL_ERROR("Failed to parse metatile height.\n");

            if (options.metatileHeight < 1)
                FATAL_ERROR("metatile height must be positive.\n");
        }
        else if (strcmp(option, "-bitdepth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No bitdepth value following \"-bitdepth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &options.bitDepth))
                FATAL_ERROR("Failed to parse bitdepth.\n");

            if (options.bitDepth != 4 && options.bitDepth != 8)
                FATAL_ERROR("bitdepth must be either 4 or 8.\n");
        }
        else if (strcmp(option, "-clobbersize") == 0)
        {
            options.clobberSize = true;
        }
        else if (strcmp(option, "-nobyteorder") == 0)
        {
            options.byteOrder = false;
        }
        else if (strcmp(option, "-version101") == 0)
        {
            options.version101 = true;
        }
        else if (strcmp(option, "-sopc") == 0)
        {
            options.sopc = true;
        }
        else if (strcmp(option, "-scanned") == 0)
        {
            if (options.scanMode != 0)
                FATAL_ERROR("Scan mode specified more than once.\n-scanned goes back to front as in DP, -scanfronttoback goes front to back as in PtHGSS\n");
            options.scanMode = 1;
        }
        else if (strcmp(option, "-scanfronttoback") == 0)
        {
            if (options.scanMode != 0)
                FATAL_ERROR("Scan mode specified more than once.\n-scanned goes back to front as in DP, -scanfronttoback goes front to back as in PtHGSS\n");
            options.scanMode = 2;
        }
        else if (strcmp(option, "-handleempty") == 0)
        {
            options.handleEmpty = true;
        }
        else if (strcmp(option, "-bitmap") == 0)
        {
            options.bitmap = true;
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    ConvertPngToNtr(inputPath, outputPath, &options);
}

void HandlePngToGbaPaletteCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Palette palette;

    ReadPngPalette(inputPath, &palette);
    WriteGbaPalette(outputPath, &palette);
}

void HandlePngToNtrPaletteCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    struct Palette palette;
    bool ncpr = false;
    bool ir = false;
    bool nopad = false;
    int bitdepth = 0;
    int compNum = 0;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-ncpr") == 0)
        {
            ncpr = true;
        }
        else if (strcmp(option, "-ir") == 0)
        {
            ir = true;
        }
        else if (strcmp(option, "-nopad") == 0)
        {
            nopad = true;
        }
        else if (strcmp(option, "-bitdepth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No bitdepth following \"-bitdepth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &bitdepth))
                FATAL_ERROR("Failed to parse bitdepth.\n");

            if (bitdepth != 4 && bitdepth != 8)
                FATAL_ERROR("Bitdepth must be 4 or 8.\n");
        }
        else if (strcmp(option, "-comp") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No compression value following \"-comp\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &compNum))
                FATAL_ERROR("Failed to parse compression value.\n");

            if (compNum > 255)
                FATAL_ERROR("Compression value must be 255 or below.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    ReadPngPalette(inputPath, &palette);
    WriteNtrPalette(outputPath, &palette, ncpr, ir, bitdepth, !nopad, compNum);
}

void HandleGbaToJascPaletteCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Palette palette;

    ReadGbaPalette(inputPath, &palette);
    WriteJascPalette(outputPath, &palette);
}

void HandleNtrToJascPaletteCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    struct Palette palette;
    int bitdepth = 0;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-bitdepth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No bitdepth following \"-bitdepth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &bitdepth))
                FATAL_ERROR("Failed to parse bitdepth.\n");

            if (bitdepth != 4 && bitdepth != 8)
                FATAL_ERROR("Bitdepth must be 4 or 8.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    ReadNtrPalette(inputPath, &palette, bitdepth, 1);
    WriteJascPalette(outputPath, &palette);
}

void HandleJascToGbaPaletteCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    int numColors = 0;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-num_colors") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No number of colors following \"-num_colors\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &numColors))
                FATAL_ERROR("Failed to parse number of colors.\n");

            if (numColors < 1)
                FATAL_ERROR("Number of colors must be positive.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    struct Palette palette;

    ReadJascPalette(inputPath, &palette);

    if (numColors != 0)
        palette.numColors = numColors;

    WriteGbaPalette(outputPath, &palette);
}

void HandleJascToNtrPaletteCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    int numColors = 0;
    bool ncpr = false;
    bool ir = false;
    bool nopad = false;
    int bitdepth = 0;
    int compNum = 0;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-num_colors") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No number of colors following \"-num_colors\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &numColors))
                FATAL_ERROR("Failed to parse number of colors.\n");

            if (numColors < 1)
                FATAL_ERROR("Number of colors must be positive.\n");
        }
        else if (strcmp(option, "-bitdepth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No bitdepth following \"-bitdepth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &bitdepth))
                FATAL_ERROR("Failed to parse bitdepth.\n");

            if (bitdepth != 4 && bitdepth != 8)
                FATAL_ERROR("Bitdepth must be 4 or 8.\n");
        }
        else if (strcmp(option, "-comp") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No compression value following \"-comp\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &compNum))
                FATAL_ERROR("Failed to parse compression value.\n");

            if (compNum > 255)
                FATAL_ERROR("Compression value must be 255 or below.\n");
        }
        else if (strcmp(option, "-ncpr") == 0)
        {
            ncpr = true;
        }
        else if (strcmp(option, "-ir") == 0)
        {
            ir = true;
        }
        else if (strcmp(option, "-nopad") == 0)
        {
            nopad = true;
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    struct Palette palette;

    ReadJascPalette(inputPath, &palette);

    if (numColors != 0)
        palette.numColors = numColors;

    WriteNtrPalette(outputPath, &palette, ncpr, ir, bitdepth, !nopad, compNum);
}

void HandleLatinFontToPngCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Image image;

    ReadLatinFont(inputPath, &image);
    WritePng(outputPath, &image);

    FreeImage(&image);
}

void HandlePngToLatinFontCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Image image;

    image.bitDepth = 2;

    ReadPng(inputPath, &image);
    WriteLatinFont(outputPath, &image);

    FreeImage(&image);
}

void HandleHalfwidthJapaneseFontToPngCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Image image;

    ReadHalfwidthJapaneseFont(inputPath, &image);
    WritePng(outputPath, &image);

    FreeImage(&image);
}

void HandlePngToHalfwidthJapaneseFontCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Image image;

    image.bitDepth = 2;

    ReadPng(inputPath, &image);
    WriteHalfwidthJapaneseFont(outputPath, &image);

    FreeImage(&image);
}

void HandleFullwidthJapaneseFontToPngCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Image image;

    ReadFullwidthJapaneseFont(inputPath, &image);
    WritePng(outputPath, &image);

    FreeImage(&image);
}

void HandlePngToFullwidthJapaneseFontCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    struct Image image;

    image.bitDepth = 2;

    ReadPng(inputPath, &image);
    WriteFullwidthJapaneseFont(outputPath, &image);

    FreeImage(&image);
}

void HandleLZCompressCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    int overflowSize = 0;
    int minDistance = 2; // default, for compatibility with LZ77UnCompVram()

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-overflow") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No size following \"-overflow\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &overflowSize))
                FATAL_ERROR("Failed to parse overflow size.\n");

            if (overflowSize < 1)
                FATAL_ERROR("Overflow size must be positive.\n");
        }
        else if (strcmp(option, "-search") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No size following \"-overflow\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &minDistance))
                FATAL_ERROR("Failed to parse LZ min search distance.\n");

            if (minDistance < 1)
                FATAL_ERROR("LZ min search distance must be positive.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    // The overflow option allows a quirk in some of Ruby/Sapphire's tilesets
    // to be reproduced. It works by appending a number of zeros to the data
    // before compressing it and then amending the LZ header's size field to
    // reflect the expected size. This will cause an overflow when decompressing
    // the data.

    int fileSize;
    unsigned char *buffer = ReadWholeFileZeroPadded(inputPath, &fileSize, overflowSize);

    int compressedSize;
    unsigned char *compressedData = LZCompress(buffer, fileSize + overflowSize, &compressedSize, minDistance);

    compressedData[1] = (unsigned char)fileSize;
    compressedData[2] = (unsigned char)(fileSize >> 8);
    compressedData[3] = (unsigned char)(fileSize >> 16);

    free(buffer);

    WriteWholeFile(outputPath, compressedData, compressedSize);

    free(compressedData);
}

void HandleLZDecompressCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    int fileSize;
    unsigned char *buffer = ReadWholeFile(inputPath, &fileSize);

    int uncompressedSize;
    unsigned char *uncompressedData = LZDecompress(buffer, fileSize, &uncompressedSize);

    free(buffer);

    WriteWholeFile(outputPath, uncompressedData, uncompressedSize);

    free(uncompressedData);
}

void HandleRLCompressCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    int fileSize;
    unsigned char *buffer = ReadWholeFile(inputPath, &fileSize);

    int compressedSize;
    unsigned char *compressedData = RLCompress(buffer, fileSize, &compressedSize);

    free(buffer);

    WriteWholeFile(outputPath, compressedData, compressedSize);

    free(compressedData);
}

void HandleRLDecompressCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    int fileSize;
    unsigned char *buffer = ReadWholeFile(inputPath, &fileSize);

    int uncompressedSize;
    unsigned char *uncompressedData = RLDecompress(buffer, fileSize, &uncompressedSize);

    free(buffer);

    WriteWholeFile(outputPath, uncompressedData, uncompressedSize);

    free(uncompressedData);
}

void HandleHuffCompressCommand(char *inputPath, char *outputPath, int argc, char **argv)
{
    int fileSize;
    int bitDepth = 4;

    for (int i = 3; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-depth") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No size following \"-depth\".\n");

            i++;

            if (!ParseNumber(argv[i], NULL, 10, &bitDepth))
                FATAL_ERROR("Failed to parse bit depth.\n");

            if (bitDepth != 4 && bitDepth != 8)
                FATAL_ERROR("GBA only supports bit depth of 4 or 8.\n");
        }
        else
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
    }

    unsigned char *buffer = ReadWholeFile(inputPath, &fileSize);

    int compressedSize;
    unsigned char *compressedData = HuffCompress(buffer, fileSize, &compressedSize, bitDepth);

    free(buffer);

    WriteWholeFile(outputPath, compressedData, compressedSize);

    free(compressedData);
}

void HandleHuffDecompressCommand(char *inputPath, char *outputPath, int argc UNUSED, char **argv UNUSED)
{
    int fileSize;
    unsigned char *buffer = ReadWholeFile(inputPath, &fileSize);

    int uncompressedSize;
    unsigned char *uncompressedData = HuffDecompress(buffer, fileSize, &uncompressedSize);

    free(buffer);

    WriteWholeFile(outputPath, uncompressedData, uncompressedSize);

    free(uncompressedData);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        FATAL_ERROR("Usage: nitrogfx INPUT_PATH OUTPUT_PATH [options...]\n");

    struct CommandHandler handlers[] =
    {
        { "1bpp", "png", HandleGbaToPngCommand },
        { "4bpp", "png", HandleGbaToPngCommand },
        { "8bpp", "png", HandleGbaToPngCommand },
        { "nbfc", "png", HandleGbaToPngCommand },
        { "ncgr", "png", HandleNtrToPngCommand },
        { "png", "1bpp", HandlePngToGbaCommand },
        { "png", "4bpp", HandlePngToGbaCommand },
        { "png", "nbfc", HandlePngToGbaCommand },
        { "png", "8bpp", HandlePngToGbaCommand },
        { "png", "ncgr", HandlePngToNtrCommand },
        { "png", "gbapal", HandlePngToGbaPaletteCommand },
        { "png", "nbfp", HandlePngToGbaPaletteCommand },
        { "png", "nclr", HandlePngToNtrPaletteCommand },
        { "gbapal", "pal", HandleGbaToJascPaletteCommand },
        { "nclr", "pal", HandleNtrToJascPaletteCommand },
        { "ncpr", "pal", HandleNtrToJascPaletteCommand },
        { "pal", "gbapal", HandleJascToGbaPaletteCommand },
        { "pal", "nclr", HandleJascToNtrPaletteCommand },
        { "latfont", "png", HandleLatinFontToPngCommand },
        { "png", "latfont", HandlePngToLatinFontCommand },
        { "hwjpnfont", "png", HandleHalfwidthJapaneseFontToPngCommand },
        { "png", "hwjpnfont", HandlePngToHalfwidthJapaneseFontCommand },
        { "fwjpnfont", "png", HandleFullwidthJapaneseFontToPngCommand },
        { "png", "fwjpnfont", HandlePngToFullwidthJapaneseFontCommand },
        { NULL, "huff", HandleHuffCompressCommand },
        { NULL, "lz", HandleLZCompressCommand },
        { "huff", NULL, HandleHuffDecompressCommand },
        { "lz", NULL, HandleLZDecompressCommand },
        { NULL, "rl", HandleRLCompressCommand },
        { "rl", NULL, HandleRLDecompressCommand },
        { NULL, NULL, NULL }
    };

    char *inputPath = argv[1];
    char *outputPath = argv[2];
    char *inputFileExtension = GetFileExtension(inputPath);
    char *outputFileExtension = GetFileExtension(outputPath);

    if (inputFileExtension == NULL)
        FATAL_ERROR("Input file \"%s\" has no extension.\n", inputPath);

    if (outputFileExtension == NULL)
        FATAL_ERROR("Output file \"%s\" has no extension.\n", outputPath);

    for (int i = 0; handlers[i].function != NULL; i++)
    {
        if ((handlers[i].inputFileExtension == NULL || strcmp(handlers[i].inputFileExtension, inputFileExtension) == 0)
            && (handlers[i].outputFileExtension == NULL || strcmp(handlers[i].outputFileExtension, outputFileExtension) == 0))
        {
            handlers[i].function(inputPath, outputPath, argc, argv);
            return 0;
        }
    }

    FATAL_ERROR("Don't know how to convert \"%s\" to \"%s\".\n", inputPath, outputPath);
}
