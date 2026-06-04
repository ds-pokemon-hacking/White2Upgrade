#include "util/filesystem.h"

namespace w2u {
    using FInitFn = void (*)(FSFile *file);
    using RomFsOpenFn = b32 (*)(FSFile *file, const char *fileName);
    using RomFsReadFn = u32 (*)(FSFile *file, void *buffer, u32 size);
    using RomFsCloseFn = void (*)(FSFile *file);
    using RomFsSeekFn = b32 (*)(FSFile *file, u32 offset, SeekOrigin origin);
    using RomFsTellFn = u32 (*)(FSFile *file);

    static inline void finitAbs(FSFile *file) {
        reinterpret_cast<FInitFn>(0x02070CA9)(file);
    }

    static inline b32 romfs_fopenAbs(FSFile *file, const char *fileName) {
        return reinterpret_cast<RomFsOpenFn>(0x02070ECD)(file, fileName);
    }

    static inline u32 romfs_freadAbs(FSFile *file, void *buffer, u32 size) {
        return reinterpret_cast<RomFsReadFn>(0x02070E6D)(file, buffer, size);
    }

    static inline void romfs_fcloseAbs(FSFile *file) {
        reinterpret_cast<RomFsCloseFn>(0x02070DE1)(file);
    }

    static inline b32 romfs_fseekAbs(FSFile *file, u32 offset, SeekOrigin origin) {
        return reinterpret_cast<RomFsSeekFn>(0x02070E55)(file, offset, origin);
    }

    static inline u32 romfs_ftellAbs(FSFile *file) {
        return reinterpret_cast<RomFsTellFn>(0x02070E21)(file);
    }

    b32 ReadDataFromFile(const char* fileName, u32 bufferSize, u8 *buffer) {
        FSFile file;
        b32 successful_read;
        finitAbs(&file);
        if (!romfs_fopenAbs(&file, fileName)) {
            return false;
        }

        successful_read = romfs_freadAbs(&file, buffer, bufferSize) == bufferSize;
        romfs_fcloseAbs(&file);
        return successful_read;
    }

    u8 ReadByteFromFile(const char* fileName, const u32 idx) {
        FSFile file;
        finitAbs(&file);
        if (!romfs_fopenAbs(&file, fileName)) {
            // Type chart not found.
            return 0;
        }

        romfs_fseekAbs(&file, 0, IO_SEEK_END);
        u32 fileSize = romfs_ftellAbs(&file);
        romfs_fseekAbs(&file, 0, IO_SEEK_SET);
        if (idx >= fileSize) {
            // Index is out of bounds.
            return 0;
        }

        u8 output = 0;
        if (romfs_fseekAbs(&file, idx, IO_SEEK_CUR)) {
            if (romfs_freadAbs(&file, &output, 1) != 1) {
                // Error while reading.
                output = 0;
            }
        }
        romfs_fcloseAbs(&file);
        return output;
    }
}
