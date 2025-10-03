#include "swan/swantypes.h"
#include "gfl/fs/gfl_archive.h"
#include "Species.h"

#define REGIONAL_DEX_FILE_INDEX 826

namespace w2u {
    namespace pml {
        extern "C" b32 PokeDex_IsCaught(void *pDexAddress, u16 species);
        extern "C" b32 PokeDex_IsSeen(void *pDexAddress, u16 species);
        extern "C" b32 PML_PkmIsRegionalDexExclude(u32 species);
        extern "C" b32 PML_PkmIsNationalDexExclude(u32 species);
        extern "C" u16 *PML_PersonalLoadRegionalDexTable(HeapID heapId, u16 *regionalDexCount);

        extern ArcTool **g_PMLPersonalArcBW2 = (ArcTool **)0x2141428;

        extern "C" bool THUMB_BRANCH_PML_PkmIsBadMonsNo(u32 species) {
            // A "Bad" Pokemon is a Pokemon that's outside of the range (or 0).
            return !species || species > SPECIES_CNT;
        }

        extern "C" u16 *THUMB_BRANCH_PML_PersonalLoadRegionalDexTable(HeapID heapId, u16 *regionalDexCount) {
            u16 v3 = 0;
            u32 fileLength;
            u16 *result = (u16 *)GFL_ArcToolReadHeapNewLZGetLen(*g_PMLPersonalArcBW2, REGIONAL_DEX_FILE_INDEX, 0, heapId, &fileLength);
            u32 v5 = (fileLength << 15 >> 16);
            if (regionalDexCount) {
                for (u32 i = 0; i < v5; i++) {
                    if (*(result + i) != 999) {
                        ++v3;
                    }
                }
                *regionalDexCount = v3;
            }
            return result;
        }

        extern "C" u16 THUMB_BRANCH_PokeDex_GetCaughtNoNational(void *pDexAddress) {
            u16 caught_number = 0;
            for (s32 i = 1; i <= SPECIES_CNT; ++i) {
                caught_number += PokeDex_IsCaught(pDexAddress, i);
            }
            return caught_number;
        }

        extern "C" u32 THUMB_BRANCH_PokeDex_GetSeenNoUnovaPermissive(void *pDexAddress, HeapID heapId) {
            u32 v3 = 0;
            u16 *RegionalDexTable = (u16 *)PML_PersonalLoadRegionalDexTable(heapId, 0);
            for (u32 i = 1; i <= SPECIES_CNT; i++) {
                if (PokeDex_IsSeen(pDexAddress, i) && RegionalDexTable[i] != 999 && PML_PkmIsRegionalDexExclude(i) ){
                    v3++;
                }
            }
            GFL_HeapFree(RegionalDexTable);
            return v3;
        }

        extern "C" u32 THUMB_BRANCH_PokeDex_GetCaughtNoUnova(void *pDexAddress, HeapID heapId) {
            u32 v3 = 0;
            u16 *RegionalDexTable = (u16 *)PML_PersonalLoadRegionalDexTable(heapId, 0);
            for (u32 i = 1; i <= SPECIES_CNT; i++){
                if (PokeDex_IsCaught(pDexAddress, i) && RegionalDexTable[i] != 999) {
                    v3++;
                }
            }
            GFL_HeapFree(RegionalDexTable);
            return v3;
        }

        extern "C" u32 THUMB_BRANCH_PokeDex_GetCaughtNoPermissive(void *pDexAddress) {
            u32 v2 = 0;
            for (s32 i = 1; i <= SPECIES_CNT; ++i) {
                if (PokeDex_IsCaught(pDexAddress, i) && PML_PkmIsNationalDexExclude(i)) {
                    v2++;
                }
            }
            return v2;
        }

        extern "C" u32 THUMB_BRANCH_PokeDex_GetCaughtNoUnovaPermissive(void *pDexAddress, HeapID heapId) {
            u32 v3 = 0;
            u16 *RegionalDexTable = (u16 *)PML_PersonalLoadRegionalDexTable(heapId, 0);
            for (u32 i = 1; i <= SPECIES_CNT; i++) {
                if (PokeDex_IsCaught(pDexAddress, i) && RegionalDexTable[i] != 999 && PML_PkmIsRegionalDexExclude(i)) {
                    v3++;
                }
            }
            GFL_HeapFree(RegionalDexTable);
            return v3;
        }

        extern "C" u16 THUMB_BRANCH_PokeDex_GetSeenNoNational(void *pDexAddress) {
            u16 v2 = 0;
            for (s32 i = 1; i <= SPECIES_CNT; ++i) {
                if (PokeDex_IsSeen(pDexAddress, i)) {
                    ++v2;
                }
            }
            return v2;
        }

        extern "C" u32 THUMB_BRANCH_PokeDex_GetSeenNoUnova(void *pDexAddress, HeapID heapId) {
            u32 v3 = 0;
            u16 *RegionalDexTable = (u16 *)PML_PersonalLoadRegionalDexTable(heapId, 0);
            for (u32 i = 1; i <= SPECIES_CNT; i++) {
                if (PokeDex_IsSeen(pDexAddress, i) && RegionalDexTable[i] != 999) {
                    v3++;
                }
            }
            GFL_HeapFree(RegionalDexTable);
            return v3;
        }
    }
}