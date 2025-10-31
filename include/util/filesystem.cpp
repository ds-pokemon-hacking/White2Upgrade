#include "util/filesystem.h"

namespace w2u {
    b32 ReadDataFromFile(const char* fileName, u32 bufferSize, u8 *buffer) {
        FSFile file;
        b32 successful_read;
        finit(&file);
        if (!romfs_fopen(&file, fileName)) {
            return false;
        }

        successful_read = romfs_fread(&file, buffer, bufferSize) == bufferSize;
        romfs_fclose(&file);
        return successful_read;
    }

    u8 ReadByteFromFile(const char* fileName, const u32 idx) {
        FSFile file;
        finit(&file);
        if (!romfs_fopen(&file, fileName)) {
            // Type chart not found.
            return 0;
        }

        romfs_fseek(&file, 0, IO_SEEK_END);
        u32 fileSize = romfs_ftell(&file);
        romfs_fseek(&file, 0, IO_SEEK_SET);
        if (idx >= fileSize) {
            // Index is out of bounds.
            return 0;
        }

        u8 output = 0;
        if (romfs_fseek(&file, idx, IO_SEEK_CUR)) {
            if (romfs_fread(&file, &output, 1) != 1) {
                // Error while reading.
                output = 0;
            }
        }
        romfs_fclose(&file);
        return output;
    }
}