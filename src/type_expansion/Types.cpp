#include "Types.h"
#include "swantypes.h"
#include "nds/fs.h"

namespace w2u {
    extern "C" {
        b32 ReadTypeChartFromFile(u8 *buffer) {
            FSFile file;
            b32 successful_read;
            finit(&file);
            if (!romfs_fopen(&file, "type_chart.bin")) {
                // Type chart not found.
                return false;
            }
            successful_read = romfs_fread(&file, buffer, 0x144) == 0x144;
            romfs_fclose(&file);
            return successful_read;
        }

        s32 THUMB_BRANCH_GetTypeEffectiveness(s32 t1, s32 t2) {
            u8 type_chart_buffer[0x144];

            if (t1 >= TYPE_COUNT_MAX || t2 >= TYPE_COUNT_MAX) {
                return RESULT_EFFECTIVE;
            }

            if (ReadTypeChartFromFile(type_chart_buffer)) {
                switch (type_chart_buffer[TYPE_COUNT_MAX * t1 + t2]) {
                    case NOT_EFFECTIVE: {
                        return RESULT_NOT_EFFECTIVE;
                    }
                    case NOT_VERY_EFFECTIVE: {
                        return RESULT_NOT_VERY_EFFECTIVE;
                    }
                    case EFFECTIVE: {
                        return RESULT_EFFECTIVE;
                    }
                    case SUPER_EFFECTIVE: {
                        return RESULT_SUPER_EFFECTIVE;
                    }
                    default: {
                        return RESULT_NOT_EFFECTIVE;
                    }
                }            
            }

            return RESULT_EFFECTIVE;
        }

        s32 THUMB_BRANCH_GetTypeWeaknesses(s32 t1, u8 *weakness_buffer) {
            u8 type_chart_buffer[0x144];
            if (t1 >= TYPE_COUNT_MAX) {
                return 0;
            }
            if (ReadTypeChartFromFile(type_chart_buffer)) {
                u32 weakness_buffer_index = 0, row = TYPE_COUNT_MAX * t1;
                for (u32 i = 0; i < TYPE_COUNT_MAX; ++i) {
                    if (!type_chart_buffer[row + i] || type_chart_buffer[row + i] == NOT_VERY_EFFECTIVE) {
                        weakness_buffer[weakness_buffer_index++] = i;
                    }
                }
                return weakness_buffer_index;
            }
            return 0;
        }
    }
}