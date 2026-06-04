#include "Species.h"
#include "nds/fs.h"
#include "pwan_types.h"
#include "string.h"
#include "w2u_pwan_config.h"

#define W2U_PWAN_MAGIC 0x4E415750u
#define W2U_FRAME_BYTES 0x1200u
#define W2U_FRAME_VRAM_OFFSET 0x7000u
#define W2U_OBJ_1D_128K_BLOCK_BYTES 128u
#define W2U_TILE_BASE (W2U_FRAME_VRAM_OFFSET / W2U_OBJ_1D_128K_BLOCK_BYTES)
#define W2U_OBJ_PLT 15u
#define W2U_OAM_BASE ((volatile u16 *)0x07000000)
#define W2U_OBJ_VRAM ((volatile u16 *)0x06400000)
#define W2U_OBJ_PLTT ((volatile u16 *)0x05000200)
#define W2U_OAM_INDEX 0u
#define W2U_REG_DISPCNT ((volatile u32 *)0x04000000)
#define W2U_VRAMCNT_A ((volatile u8 *)0x04000240)
#define W2U_VRAMCNT_B ((volatile u8 *)0x04000241)
#define W2U_VRAMCNT_C ((volatile u8 *)0x04000242)
#define W2U_VRAMCNT_D ((volatile u8 *)0x04000243)
#define W2U_SUMMARY_PROFILE_MAGIC 0x46525053u
#define W2U_SUMMARY_PROFILE_VERSION 19u
#define W2U_SUMMARY_SETTLE_FRAMES 2u
#define W2U_SUMMARY_NO_ASSET_HOLD_FRAMES 40u
#define W2U_SUMMARY_BASE_Y 40
#define W2U_SUMMARY_BASELINE_BOTTOM 82
#define W2U_MAIN_RAM_START 0x02000000u
#define W2U_MAIN_RAM_END 0x02400000u
#define W2U_MCSS_POS_OFFSET 0xe0u
#define W2U_MCSS_OFFSCREEN_POS 0x00800000u

namespace w2u {
namespace summary_anim {

struct PwanHeader {
    u32 magic;
    u16 version;
    u16 width;
    u16 height;
    u16 bpp;
    u16 frameCount;
    u16 timelineCount;
    u32 totalTicks;
    u32 frameBytes;
    u32 paletteColors;
    u32 paletteOffset;
    u32 timelineOffset;
    u32 frameOffset;
};

struct PwanTimelineEntry {
    u16 frame;
    u16 ticks;
};

#define W2U_PWAN_CONFIG_PATH "pokeweb_pwan/config.bin"
#define W2U_PWAN_CONFIG_MAGIC 0x434E5750u
#define W2U_PWAN_CONFIG_VERSION 1u
#define W2U_PWAN_MAX_OVERRIDES 500u
#define W2U_PWAN_ASSET_COUNT (W2U_PWAN_MAX_OVERRIDES * 2u)
#define W2U_SUMMARY_CACHE_COUNT 2u
#define W2U_PWAN_PATH_BYTES 32u

struct PwanConfigHeader {
    u32 magic;
    u16 version;
    u16 count;
    u16 maxTimeline;
    u16 reserved;
    u32 entriesOffset;
};

struct PwanConfigEntry {
    u16 species;
    u16 flags;
    u16 frontIndex;
    u16 backIndex;
};

struct PstatusData {
    void *ppd;
    void *cfg;
    void *gameData;
    u8 ppt;
    u8 mode;
    u8 max;
    u8 pos;
};

struct SummaryWorkView {
    u16 heapId;
    u8 pad0[6];
    PstatusData *psData;
    u8 pad1[0x88 - 0x0C];
    void *subWork;
};

struct SummarySubWorkView {
    void *pokeMcss;
    void *pokeMcssBack;
    u32 state;
    u32 isDispFront;
};

typedef u32 SummaryAssetId;
#define ASSET_NONE 0xffffffffu

#define ASSET_COUNT W2U_PWAN_ASSET_COUNT

struct Asset {
    b32 loaded;
    b32 fileOpen;
    u32 assetId;
    FSFile file;
    PwanHeader header;
    PwanTimelineEntry timeline[128];
    u16 palette[16];
    s32 summaryYOffset;
    s32 summaryMaxBottom;
};

struct State {
    b32 active;
    u32 tick[W2U_SUMMARY_CACHE_COUNT];
    u16 copiedFrame;
    u32 copiedAsset;
    u32 paletteAsset;
    u32 observedWork;
    u32 observedSubWork;
    u32 observedPokeMcss;
    u32 observedPokeMcssBack;
    u32 observedState;
    u32 observedIsDispFront;
    u32 observedSpecies;
    u32 observedAsset;
    u32 acceptedWork;
    u32 acceptedSubWork;
    u32 acceptedPokeMcss;
    u32 acceptedPokeMcssBack;
    u32 acceptedState;
    u32 acceptedIsDispFront;
    u32 acceptedSpecies;
    u32 acceptedAsset;
    u32 acceptedPos;
    u32 stableFrames;
    u32 rawObservedWork;
    u32 rawObservedSubWork;
    u32 rawObservedPokeMcss;
    u32 rawObservedPokeMcssBack;
    u32 rawObservedState;
    u32 rawObservedIsDispFront;
    u32 rawObservedPsData;
    u32 rawObservedPpt;
    u32 rawObservedMode;
    u32 rawObservedMax;
    u32 rawObservedPos;
    u32 rawStableFrames;
    u32 noAssetHoldFrames;
    Asset asset[W2U_SUMMARY_CACHE_COUNT];
    u8 frame[W2U_FRAME_BYTES];
};

enum SummarySkipReason {
    SKIP_NONE = 0,
    SKIP_NULL_WORK = 1,
    SKIP_NULL_SUB_WORK = 2,
    SKIP_INVALID_PS_DATA = 3,
    SKIP_INVALID_MCSS_FRONT = 4,
    SKIP_INVALID_MCSS_BACK = 5,
    SKIP_NO_ASSET = 6,
    SKIP_LOAD_FAIL = 7,
    SKIP_SIGNATURE_CHANGED = 8,
    SKIP_NOT_SETTLED = 9,
    SKIP_PREDRAW_MISMATCH = 10,
    SKIP_DRAW_INACTIVE = 11,
    SKIP_TERM = 12,
    SKIP_RAW_SIGNATURE_CHANGED = 13,
    SKIP_RAW_NOT_SETTLED = 14,
};

enum SummaryLifecycleStage {
    STAGE_NONE = 0,
    STAGE_UPDATE_ENTER = 1,
    STAGE_UPDATE_HAVE_WORK = 2,
    STAGE_UPDATE_BEFORE_SPECIES = 3,
    STAGE_UPDATE_AFTER_SPECIES = 4,
    STAGE_UPDATE_BEFORE_LOAD = 5,
    STAGE_UPDATE_AFTER_LOAD = 6,
    STAGE_UPDATE_BEFORE_HIDE = 7,
    STAGE_UPDATE_AFTER_HIDE = 8,
    STAGE_UPDATE_ACTIVE = 9,
    STAGE_PREDRAW_ENTER = 10,
    STAGE_PREDRAW_BEFORE_HIDE = 11,
    STAGE_PREDRAW_AFTER_HIDE = 12,
    STAGE_DRAW_ENTER = 13,
    STAGE_DRAW_DONE = 14,
    STAGE_TERM_ENTER = 15,
    STAGE_TERM_DONE = 16,
};

struct SummaryAnimProfile {
    u32 magic;
    u32 version;
    u32 structSize;
    u32 updateCalls;
    u32 preDrawCalls;
    u32 drawCalls;
    u32 termCalls;
    u32 active;
    u32 nullWorkCount;
    u32 nullSubWorkCount;
    u32 lastWork;
    u32 lastSubWork;
    u32 lastPokeMcss;
    u32 lastPokeMcssBack;
    u32 lastState;
    u32 lastIsDispFront;
    u32 lastSpecies;
    u32 lastAsset;
    u32 lastPaletteAsset;
    u32 lastCopiedAsset;
    u32 lastCopiedFrame;
    u32 lastFrameForTick;
    u32 loadFailCount;
    u32 frameReadFailCount;
    u32 lastLoadFailAsset;
    u32 lastFrameReadFailAsset;
    u32 lastFrameReadFailFrame;
    u32 hideOamCalls;
    u32 drawFrameCalls;
    u32 vanillaHideCalls;
    u32 lastDispcnt;
    u32 lastVramcntA;
    u32 lastVramcntB;
    u32 lastVramcntC;
    u32 lastVramcntD;
    u32 lastOamAttr0[4];
    u32 lastOamAttr1[4];
    u32 lastOamAttr2[4];
    u32 transitionSkipCount;
    u32 settleSkipCount;
    u32 invalidPointerSkipCount;
    u32 preDrawMismatchCount;
    u32 speciesReadCalls;
    u32 lastSkipReason;
    u32 lastLifecycleStage;
    u32 lastPsData;
    u32 lastPpt;
    u32 lastMode;
    u32 lastMax;
    u32 lastPos;
    u32 stableFrames;
    u32 lastObservedWork;
    u32 lastObservedSubWork;
    u32 lastObservedPokeMcss;
    u32 lastObservedPokeMcssBack;
    u32 lastObservedState;
    u32 lastObservedIsDispFront;
    u32 lastObservedSpecies;
    u32 lastObservedAsset;
    u32 lastAcceptedWork;
    u32 lastAcceptedSubWork;
    u32 lastAcceptedPokeMcss;
    u32 lastAcceptedPokeMcssBack;
    u32 lastAcceptedState;
    u32 lastAcceptedIsDispFront;
    u32 lastAcceptedSpecies;
    u32 lastAcceptedAsset;
    u32 vanillaRestoreCalls;
    u32 lastRestoreMcss;
    u32 transitionRestoreCalls;
    u32 hiddenRehideCalls;
    u32 lastHiddenRehideMcss;
    u32 hiddenShadowKeepHiddenCalls;
    u32 lastHiddenShadowKeepHiddenMcss;
    u32 hiddenOffscreenCalls;
    u32 lastHiddenOffscreenMcss;
    u32 lastHiddenOldPosX;
    u32 lastHiddenOldPosY;
    u32 lastHiddenOldPosZ;
    u32 transitionKeepHiddenCalls;
    u32 lastTransitionKeepHiddenReason;
    u32 lastTransitionKeepHiddenMcss;
    u32 lastAssetMaxBottom;
    s32 lastAssetYOffset;
    u32 lastLoadStep;
    u32 lastLoadReadOffset;
    u32 lastLoadReadSize;
    u32 lastLoadHeaderFrameCount;
    u32 lastLoadHeaderTimelineCount;
    u32 lastLoadHeaderFrameOffset;
};

extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
volatile SummaryAnimProfile W2U_SummaryAnim_Profile = {
    W2U_SUMMARY_PROFILE_MAGIC,
    W2U_SUMMARY_PROFILE_VERSION,
    sizeof(SummaryAnimProfile),
};
#pragma GCC diagnostic pop
}

static State sState;

static char sAssetPath[W2U_PWAN_PATH_BYTES];

typedef void *(*GetCurrentPpFn)(SummaryWorkView *work);
typedef u32 (*PpGetFn)(const void *pp, int id, void *buf);
typedef void (*McssFlagFn)(void *mcss);
typedef void (*McssShadowVanishFn)(void *mcss, u8 flag);

static GetCurrentPpFn const PSTATUS_UTIL_GetCurrentPP_Fn = (GetCurrentPpFn)0x021B4DDDu;
static PpGetFn const PP_Get_Fn = (PpGetFn)0x0201CDB5u;
static McssFlagFn const MCSS_SetVanishFlag_Fn = (McssFlagFn)0x0201ADA9u;
static McssFlagFn const MCSS_ResetVanishFlag_Fn = (McssFlagFn)0x0201ADB9u;
static McssShadowVanishFn const MCSS_SetShadowVanishFlag_Fn =
    (McssShadowVanishFn)0x0201AEF9u;

static void HideOam();
static void HideVanillaMcss(void *mcss);
static void RestoreAcceptedTransitionMcss();
static void RestoreAcceptedDisplayedVanillaMcss();
static void RestoreCurrentDisplayedVanillaMcss(SummarySubWorkView *sub);

static b32 IsAlignedMainRamPtr(const void *ptr)
{
    const u32 p = (u32)ptr;
    return p >= W2U_MAIN_RAM_START && p < W2U_MAIN_RAM_END && (p & 3u) == 0;
}

static b32 SameRawObservedSignature(SummaryWorkView *work,
                                    SummarySubWorkView *sub,
                                    const PstatusData *psData)
{
    return sState.rawObservedWork == (u32)work &&
           sState.rawObservedSubWork == (u32)sub &&
           sState.rawObservedPokeMcss == (u32)sub->pokeMcss &&
           sState.rawObservedPokeMcssBack == (u32)sub->pokeMcssBack &&
           sState.rawObservedState == sub->state &&
           sState.rawObservedIsDispFront == sub->isDispFront &&
           sState.rawObservedPsData == (u32)psData &&
           sState.rawObservedPpt == psData->ppt &&
           sState.rawObservedMode == psData->mode &&
           sState.rawObservedMax == psData->max &&
           sState.rawObservedPos == psData->pos;
}

static void RecordRawObservedSignature(SummaryWorkView *work,
                                       SummarySubWorkView *sub,
                                       const PstatusData *psData)
{
    sState.rawObservedWork = (u32)work;
    sState.rawObservedSubWork = (u32)sub;
    sState.rawObservedPokeMcss = (u32)sub->pokeMcss;
    sState.rawObservedPokeMcssBack = (u32)sub->pokeMcssBack;
    sState.rawObservedState = sub->state;
    sState.rawObservedIsDispFront = sub->isDispFront;
    sState.rawObservedPsData = (u32)psData;
    sState.rawObservedPpt = psData->ppt;
    sState.rawObservedMode = psData->mode;
    sState.rawObservedMax = psData->max;
    sState.rawObservedPos = psData->pos;
}

static b32 ShouldKeepHiddenDuringSkip(SummarySkipReason reason)
{
    return reason == SKIP_RAW_SIGNATURE_CHANGED ||
           reason == SKIP_RAW_NOT_SETTLED ||
           reason == SKIP_SIGNATURE_CHANGED ||
           reason == SKIP_NOT_SETTLED ||
           reason == SKIP_PREDRAW_MISMATCH;
}

static void KeepTransitionMcssHidden(SummarySkipReason reason, SummarySubWorkView *sub)
{
    W2U_SummaryAnim_Profile.transitionKeepHiddenCalls =
        W2U_SummaryAnim_Profile.transitionKeepHiddenCalls + 1u;
    W2U_SummaryAnim_Profile.lastTransitionKeepHiddenReason = (u32)reason;

    void *acceptedFront = (void *)sState.acceptedPokeMcss;
    void *acceptedBack = (void *)sState.acceptedPokeMcssBack;
    if (IsAlignedMainRamPtr(acceptedFront)) {
        W2U_SummaryAnim_Profile.lastTransitionKeepHiddenMcss = (u32)acceptedFront;
        HideVanillaMcss(acceptedFront);
    }
    if (IsAlignedMainRamPtr(acceptedBack) && acceptedBack != acceptedFront) {
        W2U_SummaryAnim_Profile.lastTransitionKeepHiddenMcss = (u32)acceptedBack;
        HideVanillaMcss(acceptedBack);
    }
    if (sub && IsAlignedMainRamPtr(sub->pokeMcss) && sub->pokeMcss != acceptedFront &&
        sub->pokeMcss != acceptedBack) {
        W2U_SummaryAnim_Profile.lastTransitionKeepHiddenMcss = (u32)sub->pokeMcss;
        HideVanillaMcss(sub->pokeMcss);
    }
    if (sub && IsAlignedMainRamPtr(sub->pokeMcssBack) && sub->pokeMcssBack != acceptedFront &&
        sub->pokeMcssBack != acceptedBack && sub->pokeMcssBack != sub->pokeMcss) {
        W2U_SummaryAnim_Profile.lastTransitionKeepHiddenMcss = (u32)sub->pokeMcssBack;
        HideVanillaMcss(sub->pokeMcssBack);
    }
}

static void Deactivate(SummarySkipReason reason, SummarySubWorkView *sub = 0)
{
    if (ShouldKeepHiddenDuringSkip(reason)) {
        KeepTransitionMcssHidden(reason, sub);
    } else {
        RestoreAcceptedTransitionMcss();
    }
    sState.active = false;
    W2U_SummaryAnim_Profile.active = false;
    W2U_SummaryAnim_Profile.lastSkipReason = (u32)reason;
    HideOam();
}

static void DeactivateNoRestore(SummarySkipReason reason)
{
    sState.active = false;
    W2U_SummaryAnim_Profile.active = false;
    W2U_SummaryAnim_Profile.lastSkipReason = (u32)reason;
    HideOam();
}

static b32 SameObservedSignature(SummaryWorkView *work,
                                 SummarySubWorkView *sub,
                                 u16 species,
                                 SummaryAssetId assetId)
{
    return sState.observedWork == (u32)work &&
           sState.observedSubWork == (u32)sub &&
           sState.observedPokeMcss == (u32)sub->pokeMcss &&
           sState.observedPokeMcssBack == (u32)sub->pokeMcssBack &&
           sState.observedState == sub->state &&
           sState.observedIsDispFront == sub->isDispFront &&
           sState.observedSpecies == species &&
           sState.observedAsset == (u32)assetId;
}

static void RecordObservedSignature(SummaryWorkView *work,
                                    SummarySubWorkView *sub,
                                    u16 species,
                                    SummaryAssetId assetId)
{
    sState.observedWork = (u32)work;
    sState.observedSubWork = (u32)sub;
    sState.observedPokeMcss = (u32)sub->pokeMcss;
    sState.observedPokeMcssBack = (u32)sub->pokeMcssBack;
    sState.observedState = sub->state;
    sState.observedIsDispFront = sub->isDispFront;
    sState.observedSpecies = species;
    sState.observedAsset = (u32)assetId;
    W2U_SummaryAnim_Profile.lastObservedWork = sState.observedWork;
    W2U_SummaryAnim_Profile.lastObservedSubWork = sState.observedSubWork;
    W2U_SummaryAnim_Profile.lastObservedPokeMcss = sState.observedPokeMcss;
    W2U_SummaryAnim_Profile.lastObservedPokeMcssBack = sState.observedPokeMcssBack;
    W2U_SummaryAnim_Profile.lastObservedState = sState.observedState;
    W2U_SummaryAnim_Profile.lastObservedIsDispFront = sState.observedIsDispFront;
    W2U_SummaryAnim_Profile.lastObservedSpecies = sState.observedSpecies;
    W2U_SummaryAnim_Profile.lastObservedAsset = sState.observedAsset;
}

static void AcceptObservedSignature()
{
    sState.acceptedWork = sState.observedWork;
    sState.acceptedSubWork = sState.observedSubWork;
    sState.acceptedPokeMcss = sState.observedPokeMcss;
    sState.acceptedPokeMcssBack = sState.observedPokeMcssBack;
    sState.acceptedState = sState.observedState;
    sState.acceptedIsDispFront = sState.observedIsDispFront;
    sState.acceptedSpecies = sState.observedSpecies;
    sState.acceptedAsset = sState.observedAsset;
    sState.acceptedPos = sState.rawObservedPos;
    W2U_SummaryAnim_Profile.lastAcceptedWork = sState.acceptedWork;
    W2U_SummaryAnim_Profile.lastAcceptedSubWork = sState.acceptedSubWork;
    W2U_SummaryAnim_Profile.lastAcceptedPokeMcss = sState.acceptedPokeMcss;
    W2U_SummaryAnim_Profile.lastAcceptedPokeMcssBack = sState.acceptedPokeMcssBack;
    W2U_SummaryAnim_Profile.lastAcceptedState = sState.acceptedState;
    W2U_SummaryAnim_Profile.lastAcceptedIsDispFront = sState.acceptedIsDispFront;
    W2U_SummaryAnim_Profile.lastAcceptedSpecies = sState.acceptedSpecies;
    W2U_SummaryAnim_Profile.lastAcceptedAsset = sState.acceptedAsset;
}

static b32 ShouldInitializeFrontSide(SummaryWorkView *work,
                                     SummarySubWorkView *sub,
                                     const PstatusData *psData,
                                     u16 species)
{
    if (!sub || sub->isDispFront || species == SPECIES_NONE) {
        return false;
    }

    if (!sState.acceptedWork || sState.acceptedSpecies == SPECIES_NONE) {
        return true;
    }

    return sState.acceptedWork != (u32)work ||
           sState.acceptedSubWork != (u32)sub ||
           sState.acceptedPokeMcss != (u32)sub->pokeMcss ||
           sState.acceptedPokeMcssBack != (u32)sub->pokeMcssBack ||
           sState.acceptedSpecies != species ||
           sState.acceptedPos != psData->pos;
}

static void InitializeFrontSide(SummaryWorkView *work,
                                SummarySubWorkView *sub,
                                const PstatusData *psData)
{
    sub->isDispFront = true;
    W2U_SummaryAnim_Profile.lastIsDispFront = sub->isDispFront;
    RecordRawObservedSignature(work, sub, psData);
    sState.rawStableFrames = 0;
    sState.stableFrames = 0;
    W2U_SummaryAnim_Profile.stableFrames = 0;
}

static b32 AcceptedSignatureStillMatches(SummaryWorkView *work, SummarySubWorkView *sub)
{
    return sState.active &&
           sState.acceptedWork == (u32)work &&
           sState.acceptedSubWork == (u32)sub &&
           sState.acceptedPokeMcss == (u32)sub->pokeMcss &&
           sState.acceptedPokeMcssBack == (u32)sub->pokeMcssBack &&
           sState.acceptedState == sub->state &&
           sState.acceptedIsDispFront == sub->isDispFront;
}

static void ThreeDigitDecimal(u32 value, char *out)
{
    u32 hundreds = 0;
    while (value >= 100u) {
        value -= 100u;
        hundreds = hundreds + 1u;
    }

    u32 tens = 0;
    while (value >= 10u) {
        value -= 10u;
        tens = tens + 1u;
    }

    out[0] = (char)('0' + hundreds);
    out[1] = (char)('0' + tens);
    out[2] = (char)('0' + value);
}

static void WriteAssetPath(char *out, u32 assetId)
{
    const u32 assetIndex = assetId >> 1;
    out[0] = 'p';
    out[1] = 'o';
    out[2] = 'k';
    out[3] = 'e';
    out[4] = 'w';
    out[5] = 'e';
    out[6] = 'b';
    out[7] = '_';
    out[8] = 'p';
    out[9] = 'w';
    out[10] = 'a';
    out[11] = 'n';
    out[12] = '/';
    ThreeDigitDecimal(assetIndex, out + 13);
    out[16] = '_';
    if ((assetId & 1u) == 0) {
        out[17] = 'f';
        out[18] = 'r';
        out[19] = 'o';
        out[20] = 'n';
        out[21] = 't';
        out[22] = '.';
        out[23] = 'p';
        out[24] = 'w';
        out[25] = 'a';
        out[26] = 'n';
        out[27] = 0;
    } else {
        out[17] = 'b';
        out[18] = 'a';
        out[19] = 'c';
        out[20] = 'k';
        out[21] = '.';
        out[22] = 'p';
        out[23] = 'w';
        out[24] = 'a';
        out[25] = 'n';
        out[26] = 0;
    }
}

static const char *PathForAsset(SummaryAssetId assetId)
{
    if ((u32)assetId >= ASSET_COUNT) return 0;
    WriteAssetPath(sAssetPath, (u32)assetId);
    return sAssetPath;
}

static b32 ReadPathRange(const char *path, u32 offset, void *buffer, u32 size)
{
    if (!path) return false;
    W2U_SummaryAnim_Profile.lastLoadReadOffset = offset;
    W2U_SummaryAnim_Profile.lastLoadReadSize = size;
    FSFile file;
    finit(&file);
    if (!romfs_fopen(&file, path)) {
        return false;
    }
    if (!romfs_fseek(&file, offset, IO_SEEK_SET)) {
        romfs_fclose(&file);
        return false;
    }
    const b32 ok = romfs_fread(&file, buffer, size) == size;
    romfs_fclose(&file);
    return ok;
}

static u32 CacheSlotForAsset(SummaryAssetId assetId)
{
    return ((u32)assetId) & 1u;
}

static void CloseAssetFile(Asset *asset)
{
    if (asset->fileOpen) {
        romfs_fclose(&asset->file);
        asset->fileOpen = false;
    }
}

static b32 OpenAssetFile(Asset *asset, SummaryAssetId assetId)
{
    CloseAssetFile(asset);
    const char *path = PathForAsset(assetId);
    if (!path) return false;
    finit(&asset->file);
    if (!romfs_fopen(&asset->file, path)) {
        return false;
    }
    asset->fileOpen = true;
    return true;
}

static b32 ReadAssetRange(Asset *asset, u32 offset, void *buffer, u32 size)
{
    if (!asset->fileOpen) return false;
    W2U_SummaryAnim_Profile.lastLoadReadOffset = offset;
    W2U_SummaryAnim_Profile.lastLoadReadSize = size;
    if (!romfs_fseek(&asset->file, offset, IO_SEEK_SET)) {
        return false;
    }
    return romfs_fread(&asset->file, buffer, size) == size;
}

static s32 FindFrameBottom(const u8 *frame)
{
    static const u32 segmentOffset[4] = {0x0000, 0x0800, 0x0c00, 0x1000};
    static const u32 segmentY[4] = {0, 0, 64, 64};
    static const u32 segmentTilesW[4] = {8, 4, 8, 4};
    static const u32 segmentTilesH[4] = {8, 8, 4, 4};

    s32 bottom = -1;
    for (u32 segment = 0; segment < 4; ++segment) {
        const u8 *src = frame + segmentOffset[segment];
        for (u32 tileY = 0; tileY < segmentTilesH[segment]; ++tileY) {
            for (u32 tileX = 0; tileX < segmentTilesW[segment]; ++tileX) {
                const u8 *tile = src + ((tileY * segmentTilesW[segment] + tileX) * 32u);
                for (u32 y = 0; y < 8; ++y) {
                    const s32 globalY = (s32)(segmentY[segment] + tileY * 8u + y);
                    const u8 *row = tile + y * 4u;
                    for (u32 x = 0; x < 4; ++x) {
                        if (row[x] != 0 && globalY > bottom) {
                            bottom = globalY;
                            break;
                        }
                    }
                }
            }
        }
    }
    return bottom;
}

static b32 LoadSummaryPlacement(SummaryAssetId assetId)
{
    W2U_SummaryAnim_Profile.lastLoadStep = 50;
    Asset *asset = &sState.asset[CacheSlotForAsset(assetId)];
    const u32 offset = asset->header.frameOffset;
    W2U_SummaryAnim_Profile.lastLoadStep = 51;
    if (!ReadAssetRange(asset, offset, sState.frame, W2U_FRAME_BYTES)) {
        return false;
    }

    W2U_SummaryAnim_Profile.lastLoadStep = 52;
    const s32 maxBottom = FindFrameBottom(sState.frame);
    asset->summaryMaxBottom = maxBottom;
    asset->summaryYOffset = maxBottom > W2U_SUMMARY_BASELINE_BOTTOM
                                 ? W2U_SUMMARY_BASELINE_BOTTOM - maxBottom
                                 : 0;
    W2U_SummaryAnim_Profile.lastAssetMaxBottom =
        maxBottom >= 0 ? (u32)maxBottom : 0xffffffffu;
    W2U_SummaryAnim_Profile.lastAssetYOffset = asset->summaryYOffset;
    return true;
}

static void HideOam()
{
    W2U_SummaryAnim_Profile.hideOamCalls = W2U_SummaryAnim_Profile.hideOamCalls + 1u;
    for (u32 i = 0; i < 4; ++i) {
        volatile u16 *oam = W2U_OAM_BASE + ((W2U_OAM_INDEX + i) * 4);
        oam[0] = 192;
        oam[1] = 0;
        oam[2] = 0;
        oam[3] = 0;
    }
}

static void RecordDisplayProfile()
{
    W2U_SummaryAnim_Profile.lastDispcnt = *W2U_REG_DISPCNT;
    W2U_SummaryAnim_Profile.lastVramcntA = *W2U_VRAMCNT_A;
    W2U_SummaryAnim_Profile.lastVramcntB = *W2U_VRAMCNT_B;
    W2U_SummaryAnim_Profile.lastVramcntC = *W2U_VRAMCNT_C;
    W2U_SummaryAnim_Profile.lastVramcntD = *W2U_VRAMCNT_D;
    for (u32 i = 0; i < 4; ++i) {
        volatile u16 *oam = W2U_OAM_BASE + ((W2U_OAM_INDEX + i) * 4);
        W2U_SummaryAnim_Profile.lastOamAttr0[i] = oam[0];
        W2U_SummaryAnim_Profile.lastOamAttr1[i] = oam[1];
        W2U_SummaryAnim_Profile.lastOamAttr2[i] = oam[2];
    }
}

static void CopyPalette(SummaryAssetId assetId)
{
    Asset *asset = &sState.asset[CacheSlotForAsset(assetId)];
    for (u32 i = 0; i < 16; ++i) {
        W2U_OBJ_PLTT[W2U_OBJ_PLT * 16 + i] = asset->palette[i];
    }
    sState.paletteAsset = assetId;
}

static b32 LoadAsset(SummaryAssetId assetId)
{
    W2U_SummaryAnim_Profile.lastLoadStep = 1;
    if (assetId >= ASSET_COUNT) {
        return false;
    }

    W2U_SummaryAnim_Profile.lastLoadStep = 2;
    const u32 slot = CacheSlotForAsset(assetId);
    Asset *asset = &sState.asset[slot];
    if (asset->loaded && asset->fileOpen && asset->assetId == (u32)assetId) {
        return true;
    }
    CloseAssetFile(asset);
    asset->loaded = false;
    asset->assetId = (u32)assetId;
    sState.tick[slot] = 0;

    W2U_SummaryAnim_Profile.lastLoadStep = 10;
    if (!OpenAssetFile(asset, assetId)) {
        return false;
    }
    if (!ReadAssetRange(asset, 0, &asset->header, sizeof(asset->header))) {
        CloseAssetFile(asset);
        return false;
    }
    W2U_SummaryAnim_Profile.lastLoadStep = 11;
    W2U_SummaryAnim_Profile.lastLoadHeaderFrameCount = asset->header.frameCount;
    W2U_SummaryAnim_Profile.lastLoadHeaderTimelineCount = asset->header.timelineCount;
    W2U_SummaryAnim_Profile.lastLoadHeaderFrameOffset = asset->header.frameOffset;
    if (asset->header.magic != W2U_PWAN_MAGIC ||
        asset->header.version != 1 ||
        asset->header.width != 96 ||
        asset->header.height != 96 ||
        asset->header.bpp != 4 ||
        asset->header.frameBytes != W2U_FRAME_BYTES ||
        asset->header.paletteColors != 16 ||
        asset->header.frameCount == 0 ||
        asset->header.timelineCount == 0 ||
        asset->header.timelineCount > 128) {
        CloseAssetFile(asset);
        return false;
    }

    W2U_SummaryAnim_Profile.lastLoadStep = 20;
    if (!ReadAssetRange(asset, asset->header.paletteOffset, asset->palette, sizeof(asset->palette))) {
        CloseAssetFile(asset);
        return false;
    }
    W2U_SummaryAnim_Profile.lastLoadStep = 30;
    if (!ReadAssetRange(asset, asset->header.timelineOffset, asset->timeline,
                        asset->header.timelineCount * sizeof(PwanTimelineEntry))) {
        CloseAssetFile(asset);
        return false;
    }
    W2U_SummaryAnim_Profile.lastLoadStep = 40;
    if (!LoadSummaryPlacement(assetId)) {
        CloseAssetFile(asset);
        return false;
    }

    W2U_SummaryAnim_Profile.lastLoadStep = 60;
    asset->loaded = true;
    return true;
}

static u16 FrameForTick(const Asset *asset, u32 tick)
{
    u32 at = 0;
    for (u32 i = 0; i < asset->header.timelineCount; ++i) {
        at += asset->timeline[i].ticks;
        if (tick < at) {
            const u16 frame = asset->timeline[i].frame;
            return frame < asset->header.frameCount ? frame : 0;
        }
    }
    const u16 frame = asset->timeline[asset->header.timelineCount - 1].frame;
    return frame < asset->header.frameCount ? frame : 0;
}

static b32 CopyFrameToVram(SummaryAssetId assetId, u16 frame)
{
    if (assetId >= ASSET_COUNT) {
        return false;
    }
    Asset *asset = &sState.asset[CacheSlotForAsset(assetId)];
    if (!asset->loaded || asset->assetId != (u32)assetId || frame >= asset->header.frameCount) {
        return false;
    }
    const u32 offset = asset->header.frameOffset + (frame * asset->header.frameBytes);
    if (!ReadAssetRange(asset, offset, sState.frame, W2U_FRAME_BYTES)) {
        W2U_SummaryAnim_Profile.frameReadFailCount = W2U_SummaryAnim_Profile.frameReadFailCount + 1u;
        W2U_SummaryAnim_Profile.lastFrameReadFailAsset = (u32)assetId;
        W2U_SummaryAnim_Profile.lastFrameReadFailFrame = frame;
        return false;
    }

    volatile u16 *dst = W2U_OBJ_VRAM + (W2U_FRAME_VRAM_OFFSET / 2);
    const u16 *src = (const u16 *)sState.frame;
    for (u32 i = 0; i < W2U_FRAME_BYTES / 2; ++i) {
        dst[i] = src[i];
    }
    sState.copiedFrame = frame;
    sState.copiedAsset = assetId;
    W2U_SummaryAnim_Profile.lastCopiedFrame = frame;
    W2U_SummaryAnim_Profile.lastCopiedAsset = (u32)assetId;
    return true;
}

static void SetObj(u32 index, u32 x, u32 y, u32 shape, u32 size, u32 tile)
{
    volatile u16 *oam = W2U_OAM_BASE + ((W2U_OAM_INDEX + index) * 4);
    oam[0] = (y & 0xffu) | (shape << 14);
    oam[1] = (x & 0x1ffu) | (size << 14);
    oam[2] = (tile & 0x3ffu) | (1u << 10) | (W2U_OBJ_PLT << 12);
    oam[3] = 0;
}

static void DrawFrame()
{
    W2U_SummaryAnim_Profile.drawFrameCalls = W2U_SummaryAnim_Profile.drawFrameCalls + 1u;
    const u32 x = 152;
    const u32 assetId = sState.acceptedAsset < ASSET_COUNT ? sState.acceptedAsset : sState.copiedAsset;
    const Asset *asset = assetId < ASSET_COUNT ? &sState.asset[CacheSlotForAsset(assetId)] : 0;
    const s32 yOffset = asset && asset->loaded && asset->assetId == assetId ? asset->summaryYOffset : 0;
    const u32 y = (u32)(W2U_SUMMARY_BASE_Y + yOffset);
    SetObj(0, x, y, 0, 3, W2U_TILE_BASE);
    SetObj(1, x + 64, y, 2, 3, W2U_TILE_BASE + (0x0800u / W2U_OBJ_1D_128K_BLOCK_BYTES));
    SetObj(2, x, y + 64, 1, 3, W2U_TILE_BASE + (0x0c00u / W2U_OBJ_1D_128K_BLOCK_BYTES));
    SetObj(3, x + 64, y + 64, 0, 2, W2U_TILE_BASE + (0x1000u / W2U_OBJ_1D_128K_BLOCK_BYTES));
}

static void HideVanillaMcss(void *mcss)
{
    if (!mcss) {
        return;
    }
    W2U_SummaryAnim_Profile.vanillaHideCalls = W2U_SummaryAnim_Profile.vanillaHideCalls + 1u;
    MCSS_SetVanishFlag_Fn(mcss);
    MCSS_SetShadowVanishFlag_Fn(mcss, true);
}

static void RestoreVanillaMcss(void *mcss)
{
    if (!IsAlignedMainRamPtr(mcss)) {
        return;
    }
    W2U_SummaryAnim_Profile.vanillaRestoreCalls =
        W2U_SummaryAnim_Profile.vanillaRestoreCalls + 1u;
    W2U_SummaryAnim_Profile.lastRestoreMcss = (u32)mcss;
    MCSS_ResetVanishFlag_Fn(mcss);
    MCSS_SetShadowVanishFlag_Fn(mcss, false);
}

static void RestoreHiddenSpriteKeepShadowHidden(void *mcss)
{
    if (!IsAlignedMainRamPtr(mcss)) {
        return;
    }
    W2U_SummaryAnim_Profile.vanillaRestoreCalls =
        W2U_SummaryAnim_Profile.vanillaRestoreCalls + 1u;
    W2U_SummaryAnim_Profile.lastRestoreMcss = (u32)mcss;
    W2U_SummaryAnim_Profile.hiddenShadowKeepHiddenCalls =
        W2U_SummaryAnim_Profile.hiddenShadowKeepHiddenCalls + 1u;
    W2U_SummaryAnim_Profile.lastHiddenShadowKeepHiddenMcss = (u32)mcss;
    MCSS_ResetVanishFlag_Fn(mcss);
    MCSS_SetShadowVanishFlag_Fn(mcss, true);
}

static void MoveHiddenMcssOffscreen(void *mcss)
{
    if (!IsAlignedMainRamPtr(mcss)) {
        return;
    }

    volatile u32 *pos = (volatile u32 *)((u8 *)mcss + W2U_MCSS_POS_OFFSET);
    W2U_SummaryAnim_Profile.hiddenOffscreenCalls =
        W2U_SummaryAnim_Profile.hiddenOffscreenCalls + 1u;
    W2U_SummaryAnim_Profile.lastHiddenOffscreenMcss = (u32)mcss;
    W2U_SummaryAnim_Profile.lastHiddenOldPosX = pos[0];
    W2U_SummaryAnim_Profile.lastHiddenOldPosY = pos[1];
    W2U_SummaryAnim_Profile.lastHiddenOldPosZ = pos[2];
    pos[0] = W2U_MCSS_OFFSCREEN_POS;
    pos[1] = W2U_MCSS_OFFSCREEN_POS;
}

static void RestoreAcceptedTransitionMcss()
{
    if (!sState.active) {
        return;
    }

    void *displayed = sState.acceptedIsDispFront ? (void *)sState.acceptedPokeMcss
                                                 : (void *)sState.acceptedPokeMcssBack;
    void *hidden = sState.acceptedIsDispFront ? (void *)sState.acceptedPokeMcssBack
                                              : (void *)sState.acceptedPokeMcss;
    W2U_SummaryAnim_Profile.transitionRestoreCalls =
        W2U_SummaryAnim_Profile.transitionRestoreCalls + 1u;
    RestoreVanillaMcss(displayed);
    RestoreHiddenSpriteKeepShadowHidden(hidden);
    MoveHiddenMcssOffscreen(hidden);
}

static void RestoreAcceptedDisplayedVanillaMcss()
{
    if (!sState.active) {
        return;
    }
    const u32 mcss = sState.acceptedIsDispFront ? sState.acceptedPokeMcss
                                                : sState.acceptedPokeMcssBack;
    RestoreVanillaMcss((void *)mcss);
}

static void RestoreCurrentDisplayedVanillaMcss(SummarySubWorkView *sub)
{
    if (!sub) {
        return;
    }

    void *displayed = sub->isDispFront ? sub->pokeMcss : sub->pokeMcssBack;
    void *hidden = sub->isDispFront ? sub->pokeMcssBack : sub->pokeMcss;
    RestoreVanillaMcss(displayed);
    if (hidden != displayed) {
        HideVanillaMcss(hidden);
    }
}

static b32 CurrentMcssReusesAccepted(SummarySubWorkView *sub)
{
    if (!sub || sState.acceptedAsset >= ASSET_COUNT) {
        return false;
    }
    const u32 front = (u32)sub->pokeMcss;
    const u32 back = (u32)sub->pokeMcssBack;
    return front == sState.acceptedPokeMcss ||
           front == sState.acceptedPokeMcssBack ||
           back == sState.acceptedPokeMcss ||
           back == sState.acceptedPokeMcssBack;
}

static void HideCurrentAndAcceptedMcss(SummarySubWorkView *sub)
{
    KeepTransitionMcssHidden(SKIP_NO_ASSET, sub);
}

static void ClearAcceptedPwan()
{
    sState.acceptedAsset = ASSET_NONE;
    sState.acceptedSpecies = SPECIES_NONE;
    sState.acceptedPokeMcss = 0;
    sState.acceptedPokeMcssBack = 0;
    sState.acceptedWork = 0;
    sState.acceptedSubWork = 0;
}

static u16 GetCurrentSpecies(SummaryWorkView *work)
{
    void *pp = PSTATUS_UTIL_GetCurrentPP_Fn(work);
    if (!pp) {
        return SPECIES_NONE;
    }
    return (u16)PP_Get_Fn(pp, 5, 0);
}

static SummaryAssetId GetAssetForSpeciesSide(u16 species, b32 isFront)
{
    const u32 assetId = w2u::pwan::GetAssetForSpeciesSide(species, isFront);
    return assetId == W2U_PWAN_CONFIG_ASSET_NONE ? ASSET_NONE : (SummaryAssetId)assetId;
}

extern "C" void W2U_SummaryAnim_Update(void *rawWork)
{
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_ENTER;
    W2U_SummaryAnim_Profile.updateCalls = W2U_SummaryAnim_Profile.updateCalls + 1u;
    SummaryWorkView *work = (SummaryWorkView *)rawWork;
    SummarySubWorkView *sub = work ? (SummarySubWorkView *)work->subWork : 0;
    W2U_SummaryAnim_Profile.lastWork = (u32)work;
    W2U_SummaryAnim_Profile.lastSubWork = (u32)sub;
    if (!work || !sub) {
        if (!work) {
            W2U_SummaryAnim_Profile.nullWorkCount = W2U_SummaryAnim_Profile.nullWorkCount + 1u;
            Deactivate(SKIP_NULL_WORK);
            return;
        }
        if (work && !sub) {
            W2U_SummaryAnim_Profile.nullSubWorkCount = W2U_SummaryAnim_Profile.nullSubWorkCount + 1u;
            Deactivate(SKIP_NULL_SUB_WORK);
            return;
        }
    }
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_HAVE_WORK;
    W2U_SummaryAnim_Profile.lastPokeMcss = (u32)sub->pokeMcss;
    W2U_SummaryAnim_Profile.lastPokeMcssBack = (u32)sub->pokeMcssBack;
    W2U_SummaryAnim_Profile.lastState = sub->state;
    W2U_SummaryAnim_Profile.lastIsDispFront = sub->isDispFront;

    PstatusData *psData = work->psData;
    W2U_SummaryAnim_Profile.lastPsData = (u32)psData;
    if (!IsAlignedMainRamPtr(psData)) {
        W2U_SummaryAnim_Profile.invalidPointerSkipCount =
            W2U_SummaryAnim_Profile.invalidPointerSkipCount + 1u;
        Deactivate(SKIP_INVALID_PS_DATA);
        return;
    }
    W2U_SummaryAnim_Profile.lastPpt = psData->ppt;
    W2U_SummaryAnim_Profile.lastMode = psData->mode;
    W2U_SummaryAnim_Profile.lastMax = psData->max;
    W2U_SummaryAnim_Profile.lastPos = psData->pos;
    if (sub->pokeMcss && !IsAlignedMainRamPtr(sub->pokeMcss)) {
        W2U_SummaryAnim_Profile.invalidPointerSkipCount =
            W2U_SummaryAnim_Profile.invalidPointerSkipCount + 1u;
        Deactivate(SKIP_INVALID_MCSS_FRONT);
        return;
    }
    if (sub->pokeMcssBack && !IsAlignedMainRamPtr(sub->pokeMcssBack)) {
        W2U_SummaryAnim_Profile.invalidPointerSkipCount =
            W2U_SummaryAnim_Profile.invalidPointerSkipCount + 1u;
        Deactivate(SKIP_INVALID_MCSS_BACK);
        return;
    }

    if (!SameRawObservedSignature(work, sub, psData)) {
        RecordRawObservedSignature(work, sub, psData);
        sState.rawStableFrames = 0;
        sState.stableFrames = 0;
        W2U_SummaryAnim_Profile.stableFrames = sState.rawStableFrames;
        W2U_SummaryAnim_Profile.transitionSkipCount =
            W2U_SummaryAnim_Profile.transitionSkipCount + 1u;
        Deactivate(SKIP_RAW_SIGNATURE_CHANGED, sub);
        return;
    }
    if (sState.rawStableFrames < W2U_SUMMARY_SETTLE_FRAMES) {
        sState.rawStableFrames = sState.rawStableFrames + 1u;
        sState.stableFrames = 0;
        W2U_SummaryAnim_Profile.stableFrames = sState.rawStableFrames;
        W2U_SummaryAnim_Profile.settleSkipCount = W2U_SummaryAnim_Profile.settleSkipCount + 1u;
        Deactivate(SKIP_RAW_NOT_SETTLED, sub);
        return;
    }

    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_BEFORE_SPECIES;
    W2U_SummaryAnim_Profile.speciesReadCalls = W2U_SummaryAnim_Profile.speciesReadCalls + 1u;
    const u16 species = GetCurrentSpecies(work);
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_AFTER_SPECIES;
    if (ShouldInitializeFrontSide(work, sub, psData, species)) {
        InitializeFrontSide(work, sub, psData);
        W2U_SummaryAnim_Profile.transitionSkipCount =
            W2U_SummaryAnim_Profile.transitionSkipCount + 1u;
        Deactivate(SKIP_RAW_SIGNATURE_CHANGED, sub);
        return;
    }

    const SummaryAssetId assetId = GetAssetForSpeciesSide(species, sub->isDispFront);
    W2U_SummaryAnim_Profile.lastSpecies = species;
    W2U_SummaryAnim_Profile.lastAsset = (u32)assetId;
    if (assetId >= ASSET_COUNT) {
        if (CurrentMcssReusesAccepted(sub)) {
            if (sState.noAssetHoldFrames == 0) {
                sState.noAssetHoldFrames = W2U_SUMMARY_NO_ASSET_HOLD_FRAMES;
            }
            HideCurrentAndAcceptedMcss(sub);
            sState.noAssetHoldFrames = sState.noAssetHoldFrames - 1u;
            if (sState.noAssetHoldFrames == 0) {
                ClearAcceptedPwan();
            }
            DeactivateNoRestore(SKIP_NO_ASSET);
            return;
        }

        if (sState.noAssetHoldFrames != 0) {
            HideVanillaMcss(sub->pokeMcss);
            HideVanillaMcss(sub->pokeMcssBack);
            sState.noAssetHoldFrames = sState.noAssetHoldFrames - 1u;
            DeactivateNoRestore(SKIP_NO_ASSET);
            return;
        }

        RestoreCurrentDisplayedVanillaMcss(sub);
        ClearAcceptedPwan();
        DeactivateNoRestore(SKIP_NO_ASSET);
        return;
    }
    sState.noAssetHoldFrames = 0;

    if (!SameObservedSignature(work, sub, species, assetId)) {
        RecordObservedSignature(work, sub, species, assetId);
        sState.stableFrames = 0;
        W2U_SummaryAnim_Profile.stableFrames = sState.stableFrames;
        W2U_SummaryAnim_Profile.transitionSkipCount =
            W2U_SummaryAnim_Profile.transitionSkipCount + 1u;
        Deactivate(SKIP_SIGNATURE_CHANGED, sub);
        return;
    }
    if (sState.stableFrames < W2U_SUMMARY_SETTLE_FRAMES) {
        sState.stableFrames = sState.stableFrames + 1u;
        W2U_SummaryAnim_Profile.stableFrames = sState.stableFrames;
        W2U_SummaryAnim_Profile.settleSkipCount = W2U_SummaryAnim_Profile.settleSkipCount + 1u;
        Deactivate(SKIP_NOT_SETTLED, sub);
        return;
    }
    W2U_SummaryAnim_Profile.stableFrames = sState.stableFrames;

    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_BEFORE_LOAD;
    if (!LoadAsset(assetId)) {
        W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_AFTER_LOAD;
        W2U_SummaryAnim_Profile.loadFailCount = W2U_SummaryAnim_Profile.loadFailCount + 1u;
        W2U_SummaryAnim_Profile.lastLoadFailAsset = (u32)assetId;
        Deactivate(SKIP_LOAD_FAIL);
        return;
    }
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_AFTER_LOAD;

    const b32 wasInactive = !sState.active;
    sState.active = true;
    W2U_SummaryAnim_Profile.active = true;
    W2U_SummaryAnim_Profile.lastSkipReason = SKIP_NONE;
    AcceptObservedSignature();
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_BEFORE_HIDE;
    HideVanillaMcss(sub->pokeMcss);
    HideVanillaMcss(sub->pokeMcssBack);
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_AFTER_HIDE;

    if (wasInactive || sState.paletteAsset != assetId) {
        CopyPalette(assetId);
        W2U_SummaryAnim_Profile.lastPaletteAsset = (u32)assetId;
    }

    const u32 slot = CacheSlotForAsset(assetId);
    Asset *asset = &sState.asset[slot];
    const u32 totalTicks = asset->header.totalTicks ? asset->header.totalTicks : 1;
    if (sState.tick[slot] >= totalTicks) {
        sState.tick[slot] = 0;
    }
    const u16 frame = FrameForTick(asset, sState.tick[slot]);
    W2U_SummaryAnim_Profile.lastFrameForTick = frame;
    if (wasInactive || sState.copiedAsset != assetId || frame != sState.copiedFrame) {
        if (!CopyFrameToVram(assetId, frame)) {
            Deactivate(SKIP_LOAD_FAIL);
            return;
        }
    }
    sState.tick[slot] = sState.tick[slot] + 1u;
    if (sState.tick[slot] >= totalTicks) {
        sState.tick[slot] = 0;
    }
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_UPDATE_ACTIVE;
}

extern "C" void W2U_SummaryAnim_PreDraw(void *rawWork)
{
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_PREDRAW_ENTER;
    W2U_SummaryAnim_Profile.preDrawCalls = W2U_SummaryAnim_Profile.preDrawCalls + 1u;
    RecordDisplayProfile();
    SummaryWorkView *work = (SummaryWorkView *)rawWork;
    SummarySubWorkView *sub = work ? (SummarySubWorkView *)work->subWork : 0;
    if (!sState.active || !sub) {
        return;
    }
    if ((sub->pokeMcss && !IsAlignedMainRamPtr(sub->pokeMcss)) ||
        (sub->pokeMcssBack && !IsAlignedMainRamPtr(sub->pokeMcssBack)) ||
        !AcceptedSignatureStillMatches(work, sub)) {
        W2U_SummaryAnim_Profile.preDrawMismatchCount =
            W2U_SummaryAnim_Profile.preDrawMismatchCount + 1u;
        Deactivate(SKIP_PREDRAW_MISMATCH, sub);
        return;
    }

    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_PREDRAW_BEFORE_HIDE;
    HideVanillaMcss(sub->pokeMcss);
    HideVanillaMcss(sub->pokeMcssBack);
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_PREDRAW_AFTER_HIDE;
}

extern "C" void W2U_SummaryAnim_Draw(void *)
{
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_DRAW_ENTER;
    W2U_SummaryAnim_Profile.drawCalls = W2U_SummaryAnim_Profile.drawCalls + 1u;
    if (sState.active) {
        DrawFrame();
    } else {
        if (W2U_SummaryAnim_Profile.lastSkipReason == SKIP_NONE) {
            W2U_SummaryAnim_Profile.lastSkipReason = SKIP_DRAW_INACTIVE;
        }
        HideOam();
    }
    RecordDisplayProfile();
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_DRAW_DONE;
}

extern "C" void W2U_SummaryAnim_Term(void *)
{
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_TERM_ENTER;
    W2U_SummaryAnim_Profile.termCalls = W2U_SummaryAnim_Profile.termCalls + 1u;
    RestoreAcceptedDisplayedVanillaMcss();
    sState.active = false;
    W2U_SummaryAnim_Profile.active = false;
    W2U_SummaryAnim_Profile.lastSkipReason = SKIP_TERM;
    sState.copiedFrame = 0xffffu;
    sState.copiedAsset = ASSET_NONE;
    sState.paletteAsset = ASSET_NONE;
    sState.stableFrames = 0;
    sState.rawStableFrames = 0;
    W2U_SummaryAnim_Profile.stableFrames = 0;
    for (u32 i = 0; i < W2U_SUMMARY_CACHE_COUNT; ++i) {
        CloseAssetFile(&sState.asset[i]);
        sState.asset[i].loaded = false;
        sState.asset[i].assetId = ASSET_NONE;
        sState.tick[i] = 0;
    }
    HideOam();
    W2U_SummaryAnim_Profile.lastLifecycleStage = STAGE_TERM_DONE;
}

} // namespace summary_anim
} // namespace w2u
