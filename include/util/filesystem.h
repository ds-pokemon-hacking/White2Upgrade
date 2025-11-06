#ifndef __FILE_SYSTEM_H
#define __FILE_SYSTEM_H

#include "swantypes.h"
#include "nds/fs.h"

namespace w2u {
    b32 ReadDataFromFile(const char* fileName, u32 bufferSize, u8 *buffer);
    u8 ReadByteFromFile(const char* fileName, const u32 idx);
}

#endif