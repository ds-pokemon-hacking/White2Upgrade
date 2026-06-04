#include "Species.h"
#include "nds/fs.h"
#include "pwan_types.h"
#include "w2u_pwan_config.h"

extern "C" void *memset(void *dst, int value, unsigned int size)
{
    u8 *out = (u8 *)dst;
    const u8 byte = (u8)value;
    for (u32 i = 0; i < size; ++i) {
        out[i] = byte;
    }
    return dst;
}

#define W2U_PWAN_MAGIC 0x4E415750u
#define W2U_FRAME_BYTES 0x1200u
#define W2U_INDEP_TEX_BYTES 0x2000u
#define W2U_INDEP_TEX_SIDE 128u
#define W2U_INDEP_TEX_STRIDE_BYTES (W2U_INDEP_TEX_SIDE >> 1)
#define W2U_FRAME_VRAM_OFFSET 0x7000u
#define W2U_OBJ_1D_64K_BLOCK_BYTES 64u
#define W2U_TILE_BASE (W2U_FRAME_VRAM_OFFSET / W2U_OBJ_1D_64K_BLOCK_BYTES)
#define W2U_OBJ_PLT 15u
#define W2U_OBJ_INDEX_LOW 0u
#define W2U_OBJ_INDEX_HIGH 124u
#define W2U_OBJ_PRIORITY 1u
#define W2U_EVO_HIGH_OAM_FRAMES 120u
#define W2U_EVO_BASE_X 80u
#define W2U_EVO_BASE_Y 56u
#define W2U_OBJ_OAM ((volatile u16 *)0x07000000)
#define W2U_OBJ_VRAM ((volatile u16 *)0x06400000)
#define W2U_OBJ_PLTT ((volatile u16 *)0x05000200)
#define W2U_REG_DISPCNT ((volatile u32 *)0x04000000)
#define W2U_REG_VCOUNT ((volatile u16 *)0x04000006)
#define W2U_VRAMCNT_A ((volatile u8 *)0x04000240)
#define W2U_VRAMCNT_B ((volatile u8 *)0x04000241)
#define W2U_VRAMCNT_C ((volatile u8 *)0x04000242)
#define W2U_VRAMCNT_D ((volatile u8 *)0x04000243)
#define W2U_VRAMCNT_E ((volatile u8 *)0x04000244)
#define W2U_VRAMCNT_F ((volatile u8 *)0x04000245)
#define W2U_VRAMCNT_G ((volatile u8 *)0x04000246)
#define W2U_LCDC_TEX_VRAM ((volatile u16 *)0x06800000)
#define W2U_LCDC_TEX_PLTT ((volatile u16 *)0x06890000)
#define W2U_DISPCNT_OBJ_ENABLE (1u << 12)
#define W2U_DISPCNT_OBJ_1D_MAP (1u << 4)
#define W2U_VRAMCNT_ENABLE 0x80u
#define W2U_VRAMCNT_MST_MAIN_OBJ 0x02u
#define W2U_VRAM_LCDC_ENABLE 0x80u
#define W2U_MCSS_FLAGS_OFFSET 0x140u
#define W2U_MAIN_RAM_START 0x02000000u
#define W2U_MAIN_RAM_END 0x02400000u
#define W2U_EVO_WORK_INDEPENDENT_MGR_OFFSET 0x58u
#define W2U_INDEP_MGR_POKE0_OFFSET 0x08u
#define W2U_INDEP_MGR_POKE1_OFFSET 0x0cu
#define W2U_INDEP_POKE_CHR_PTR_OFFSET 0x00u
#define W2U_INDEP_POKE_BMP_PTR_OFFSET 0x10u
#define W2U_INDEP_POKE_TEX_ADR_OFFSET 0x14u
#define W2U_INDEP_POKE_PAL_ADR_OFFSET 0x18u
#define W2U_INDEP_POKE_PAL_PTR_OFFSET 0x04u
#define W2U_INDEP_POKE_PAL_CURR_OFFSET 0x2cu
#define W2U_INDEP_POKE_PAL_RATE_OFFSET 0x56u
#define W2U_INDEP_POKE_PAL_COLOR_OFFSET 0x4cu
#define W2U_NNS_G2D_CHAR_SZ_BYTE_OFFSET 0x10u
#define W2U_NNS_G2D_CHAR_RAW_DATA_OFFSET 0x14u
#define W2U_GFL_BMP_CHAR_PTR_OFFSET 0x00u
#define W2U_NNS_G2D_PALETTE_SZ_BYTE_OFFSET 0x08u
#define W2U_NNS_G2D_PALETTE_RAW_DATA_OFFSET 0x0cu
#define W2U_SAFE_VCOUNT_LOW 192u
#define W2U_SAFE_VCOUNT_HIGH 200u

namespace w2u {
namespace evolution_anim {

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

typedef u32 AssetId;
#define ASSET_NONE 0xffffffffu

struct Asset {
    b32 loaded;
    b32 fileOpen;
    AssetId assetId;
    FSFile file;
    PwanHeader header;
    PwanTimelineEntry timeline[128];
    u16 palette[16];
};

struct Actor {
    b32 active;
    b32 pwan;
    void *system;
    void *mcss;
    u16 species;
    u16 copiedObjFrame;
    u16 copiedIndepFrame;
    u32 tick;
    AssetId assetId;
    Asset asset;
};

enum Phase {
    PHASE_PRE_MCSS = 0,
    PHASE_INDEPENDENT = 1,
    PHASE_POST_MCSS = 2,
};

enum SkipReason {
    EVO_SKIP_NONE = 0,
    EVO_SKIP_INACTIVE = 1,
    EVO_SKIP_AFTER_NO_ASSET = 2,
    EVO_SKIP_WRONG_SYSTEM = 3,
    EVO_SKIP_BAD_MCSS = 4,
    EVO_SKIP_LOAD_FAIL = 5,
    EVO_SKIP_FRAME_READ_FAIL = 6,
    EVO_SKIP_BAD_INDEPENDENT = 7,
};

struct State {
    b32 active;
    b32 sawIndependent;
    u8 addLoopIndex;
    u8 savedVramcntE;
    b32 savedVramcntEValid;
    b32 readyToDrawObj;
    Phase phase;
    u32 mcssDrawFrames;
    Actor before;
    Actor after;
    AssetId copiedObjAsset;
    u16 copiedObjFrame;
};

struct EvoProfile {
    u32 magic;
    u32 version;
    u32 structSize;
    u32 addLoopCalls;
    u32 addSingleCalls;
    u32 drawMcssCalls;
    u32 drawIndependentCalls;
    u32 deleteCalls;
    u32 nativeMcssDrawCalls;
    u32 nativeIndependentDrawCalls;
    u32 hideNativeCalls;
    u32 objDrawCalls;
    u32 independentUploadCalls;
    u32 loadFailCount;
    u32 frameReadFailCount;
    u32 beforeSpecies;
    u32 afterSpecies;
    u32 beforeAsset;
    u32 afterAsset;
    u32 phase;
    u32 active;
    u32 skipReason;
    u32 copiedObjFrame;
    u32 beforeCopiedIndependentFrame;
    u32 afterCopiedIndependentFrame;
    u32 lastSystem;
    u32 lastMcss;
    u32 lastWork;
    u32 lastIndependentMgr;
    u32 lastPokeWork[2];
    u32 lastTexAdr[2];
    u32 lastPalAdr[2];
    u32 lastVcountBeforeWait;
    u32 lastVcountAfterWait;
};

extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
volatile EvoProfile W2U_Evolution_Profile = {
    0x52465645u,
    1u,
    sizeof(EvoProfile),
};
#pragma GCC diagnostic pop
}

static State sState;
static char sAssetPath[W2U_PWAN_PATH_BYTES];
static u8 sFrameScratch[W2U_FRAME_BYTES];
static u8 sIndepTextureScratch[W2U_INDEP_TEX_BYTES];

typedef u32 (*PpGetFn)(const void *pp, int id, void *buf);
typedef void *(*AddPokeMcssFn)(void *system, const void *pp, int dir, s32 x, s32 y, s32 z);
typedef void (*McssMainFn)(void *system);
typedef void (*McssDrawFn)(void *system);
typedef void (*McssDelFn)(void *system, void *mcss);
typedef void (*McssFlagFn)(void *mcss);
typedef void (*McssShadowVanishFn)(void *mcss, u8 flag);
typedef void (*IndependentMainFn)(void *work);
typedef void (*IndependentDrawFn)(void *work);
typedef void (*Graphic3DEndDrawFn)(void *work);

static PpGetFn const PP_Get_Fn = (PpGetFn)0x0201CD25u;
static AddPokeMcssFn const NativeAddPokeSprite_Fn = (AddPokeMcssFn)0x0201C179u;
static McssMainFn const MCSS_Main_Fn = (McssMainFn)0x02019B15u;
static McssDrawFn const MCSS_Draw_Fn = (McssDrawFn)0x02019C39u;
static McssDelFn const MCSS_Del_Fn = (McssDelFn)0x0201AAADu;
static McssFlagFn const MCSS_SetVanishFlag_Fn = (McssFlagFn)0x0201ADA9u;
static McssShadowVanishFn const MCSS_SetShadowVanishFlag_Fn =
    (McssShadowVanishFn)0x0201AEF9u;
static IndependentMainFn const EvoStaticRendererMain_Fn =
    (IndependentMainFn)0x021E57A1u;
static IndependentDrawFn const EvoStaticRendererDraw_Fn =
    (IndependentDrawFn)0x021E5801u;
static Graphic3DEndDrawFn const EvoGraphicEndDraw_Fn =
    (Graphic3DEndDrawFn)0x021E4BB9u;

static b32 IsLikelyMainRamPointer(const void *ptr)
{
    const u32 value = (u32)ptr;
    return value >= W2U_MAIN_RAM_START && value < W2U_MAIN_RAM_END;
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

static const char *PathForAsset(AssetId assetId)
{
    if (assetId >= W2U_PWAN_ASSET_COUNT) return 0;
    WriteAssetPath(sAssetPath, assetId);
    return sAssetPath;
}

static b32 ReadPathRange(const char *path, u32 offset, void *buffer, u32 size)
{
    if (!path) return false;
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

static void CloseAssetFile(Asset *asset)
{
    if (asset->fileOpen) {
        romfs_fclose(&asset->file);
        asset->fileOpen = false;
    }
}

static b32 OpenAssetFile(Asset *asset, AssetId assetId)
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
    if (!romfs_fseek(&asset->file, offset, IO_SEEK_SET)) {
        return false;
    }
    return romfs_fread(&asset->file, buffer, size) == size;
}

static AssetId GetAssetForSpeciesSide(u16 species, b32 isFront)
{
    const u32 assetId = w2u::pwan::GetAssetForSpeciesSide(species, isFront);
    return assetId == W2U_PWAN_CONFIG_ASSET_NONE ? ASSET_NONE : (AssetId)assetId;
}

static b32 LoadAsset(Actor *actor, AssetId assetId)
{
    if (assetId >= W2U_PWAN_ASSET_COUNT) {
        return false;
    }
    Asset *asset = &actor->asset;
    if (asset->loaded && asset->fileOpen && asset->assetId == assetId) {
        return true;
    }
    CloseAssetFile(asset);
    asset->loaded = false;
    asset->assetId = assetId;

    if (!OpenAssetFile(asset, assetId)) {
        return false;
    }
    if (!ReadAssetRange(asset, 0, &asset->header, sizeof(asset->header))) {
        CloseAssetFile(asset);
        return false;
    }
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

    if (!ReadAssetRange(asset, asset->header.paletteOffset, asset->palette, sizeof(asset->palette))) {
        CloseAssetFile(asset);
        return false;
    }
    if (!ReadAssetRange(asset, asset->header.timelineOffset, asset->timeline,
                        asset->header.timelineCount * sizeof(PwanTimelineEntry))) {
        CloseAssetFile(asset);
        return false;
    }
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

static void AdvanceTick(Actor *actor)
{
    const u32 totalTicks = actor->asset.header.totalTicks ?
        actor->asset.header.totalTicks : 1u;
    actor->tick = actor->tick + 1u;
    if (actor->tick >= totalTicks) {
        actor->tick = 0;
    }
}

static void ClearActor(Actor *actor)
{
    CloseAssetFile(&actor->asset);
    actor->asset.loaded = false;
    actor->asset.assetId = ASSET_NONE;
    actor->active = false;
    actor->pwan = false;
    actor->system = 0;
    actor->mcss = 0;
    actor->species = SPECIES_NONE;
    actor->copiedObjFrame = 0xffffu;
    actor->copiedIndepFrame = 0xffffu;
    actor->tick = 0;
    actor->assetId = ASSET_NONE;
}

static void ClearState()
{
    sState.active = false;
    sState.sawIndependent = false;
    sState.addLoopIndex = 0;
    sState.savedVramcntE = 0;
    sState.savedVramcntEValid = false;
    sState.readyToDrawObj = false;
    sState.phase = PHASE_PRE_MCSS;
    sState.mcssDrawFrames = 0;
    ClearActor(&sState.before);
    ClearActor(&sState.after);
    sState.copiedObjAsset = ASSET_NONE;
    sState.copiedObjFrame = 0xffffu;
}

static void UpdateActiveFlag()
{
    sState.active = sState.after.pwan;
    W2U_Evolution_Profile.active = sState.active ? 1u : 0u;
    W2U_Evolution_Profile.beforeSpecies = sState.before.species;
    W2U_Evolution_Profile.afterSpecies = sState.after.species;
    W2U_Evolution_Profile.beforeAsset = sState.before.assetId;
    W2U_Evolution_Profile.afterAsset = sState.after.assetId;
    if (!sState.active) {
        W2U_Evolution_Profile.skipReason = EVO_SKIP_AFTER_NO_ASSET;
    }
}

static u16 GetPpSpecies(const void *pp)
{
    if (!pp) return SPECIES_NONE;
    return (u16)PP_Get_Fn(pp, 5, 0);
}

static void RegisterActor(Actor *actor, void *system, void *mcss, const void *pp)
{
    actor->active = true;
    actor->system = system;
    actor->mcss = mcss;
    actor->species = GetPpSpecies(pp);
    actor->copiedObjFrame = 0xffffu;
    actor->copiedIndepFrame = 0xffffu;
    actor->tick = 0;
    actor->assetId = GetAssetForSpeciesSide(actor->species, true);
    actor->pwan = false;
    if (!system || !IsLikelyMainRamPointer(mcss) || actor->assetId >= W2U_PWAN_ASSET_COUNT) {
        return;
    }
    if (!LoadAsset(actor, actor->assetId)) {
        W2U_Evolution_Profile.loadFailCount = W2U_Evolution_Profile.loadFailCount + 1u;
        W2U_Evolution_Profile.skipReason = EVO_SKIP_LOAD_FAIL;
        actor->assetId = ASSET_NONE;
        return;
    }
    actor->pwan = true;
}

static void HideOamRange(u32 baseIndex)
{
    for (u32 i = 0; i < 4; ++i) {
        volatile u16 *oam = W2U_OBJ_OAM + ((baseIndex + i) * 4u);
        oam[0] = 192u;
        oam[1] = 0;
        oam[2] = 0;
        oam[3] = 0;
    }
}

static void HideOam()
{
    HideOamRange(W2U_OBJ_INDEX_LOW);
    HideOamRange(W2U_OBJ_INDEX_HIGH);
}

static void PrepareObjDisplay()
{
    *W2U_REG_DISPCNT = *W2U_REG_DISPCNT | W2U_DISPCNT_OBJ_ENABLE | W2U_DISPCNT_OBJ_1D_MAP;
    if (!sState.savedVramcntEValid) {
        sState.savedVramcntE = *W2U_VRAMCNT_E;
        sState.savedVramcntEValid = true;
    }
    *W2U_VRAMCNT_E = W2U_VRAMCNT_ENABLE | W2U_VRAMCNT_MST_MAIN_OBJ;
}

static void RestoreNativeVramMapping()
{
    if (sState.savedVramcntEValid) {
        *W2U_VRAMCNT_E = sState.savedVramcntE;
    }
}

static void CopyObjPalette(const Actor *actor)
{
    for (u32 i = 0; i < 16u; ++i) {
        W2U_OBJ_PLTT[W2U_OBJ_PLT * 16u + i] = actor->asset.palette[i];
    }
}

static b32 ReadFrame(Actor *actor, u16 frame)
{
    const u32 offset = actor->asset.header.frameOffset + frame * actor->asset.header.frameBytes;
    if (!ReadAssetRange(&actor->asset, offset, sFrameScratch, W2U_FRAME_BYTES)) {
        W2U_Evolution_Profile.frameReadFailCount =
            W2U_Evolution_Profile.frameReadFailCount + 1u;
        W2U_Evolution_Profile.skipReason = EVO_SKIP_FRAME_READ_FAIL;
        return false;
    }
    return true;
}

static b32 CopyObjFrame(Actor *actor, u16 frame)
{
    if (!ReadFrame(actor, frame)) return false;
    volatile u16 *dst = W2U_OBJ_VRAM + (W2U_FRAME_VRAM_OFFSET >> 1);
    const u16 *src = (const u16 *)sFrameScratch;
    for (u32 i = 0; i < (W2U_FRAME_BYTES >> 1); ++i) {
        dst[i] = src[i];
    }
    actor->copiedObjFrame = frame;
    sState.copiedObjFrame = frame;
    sState.copiedObjAsset = actor->assetId;
    W2U_Evolution_Profile.copiedObjFrame = frame;
    return true;
}

static u32 CurrentObjBaseIndex()
{
    if (sState.phase == PHASE_PRE_MCSS &&
        sState.mcssDrawFrames < W2U_EVO_HIGH_OAM_FRAMES) {
        return W2U_OBJ_INDEX_HIGH;
    }
    return W2U_OBJ_INDEX_LOW;
}

static void SetObj(u32 baseIndex, u32 index, u32 x, u32 y, u32 shape, u32 size, u32 tile)
{
    volatile u16 *oam = W2U_OBJ_OAM + ((baseIndex + index) * 4u);
    oam[0] = (u16)((y & 0xffu) | (shape << 14));
    oam[1] = (u16)((x & 0x1ffu) | (size << 14));
    oam[2] = (u16)((tile & 0x3ffu) | (W2U_OBJ_PRIORITY << 10) | (W2U_OBJ_PLT << 12));
    oam[3] = 0;
}

static void DrawObjFrame()
{
    const u32 baseIndex = CurrentObjBaseIndex();
    HideOamRange(baseIndex == W2U_OBJ_INDEX_HIGH ? W2U_OBJ_INDEX_LOW : W2U_OBJ_INDEX_HIGH);
    const u32 x = W2U_EVO_BASE_X;
    const u32 y = W2U_EVO_BASE_Y;
    SetObj(baseIndex, 0, x, y, 0, 3, W2U_TILE_BASE);
    SetObj(baseIndex, 1, x + 64u, y, 2, 3, W2U_TILE_BASE + (0x0800u >> 6));
    SetObj(baseIndex, 2, x, y + 64u, 1, 3, W2U_TILE_BASE + (0x0c00u >> 6));
    SetObj(baseIndex, 3, x + 64u, y + 64u, 0, 2, W2U_TILE_BASE + (0x1000u >> 6));
    W2U_Evolution_Profile.objDrawCalls = W2U_Evolution_Profile.objDrawCalls + 1u;
}

static void HideNativeMcss(void *mcss)
{
    if (!IsLikelyMainRamPointer(mcss)) {
        return;
    }
    MCSS_SetVanishFlag_Fn(mcss);
    MCSS_SetShadowVanishFlag_Fn(mcss, true);
    W2U_Evolution_Profile.hideNativeCalls = W2U_Evolution_Profile.hideNativeCalls + 1u;
}

static void NativeMcssDraw(void *system)
{
    RestoreNativeVramMapping();
    MCSS_Draw_Fn(system);
    W2U_Evolution_Profile.nativeMcssDrawCalls =
        W2U_Evolution_Profile.nativeMcssDrawCalls + 1u;
}

static b32 IsSafeVramUploadTime()
{
    const u16 vcount = *W2U_REG_VCOUNT;
    return vcount >= W2U_SAFE_VCOUNT_LOW && vcount <= W2U_SAFE_VCOUNT_HIGH;
}

static void WaitForSafeVramUploadTime()
{
    W2U_Evolution_Profile.lastVcountBeforeWait = *W2U_REG_VCOUNT;
    while (!IsSafeVramUploadTime()) {
    }
    W2U_Evolution_Profile.lastVcountAfterWait = *W2U_REG_VCOUNT;
}

static void ClearIndependentScratch()
{
    for (u32 i = 0; i < W2U_INDEP_TEX_BYTES; ++i) {
        sIndepTextureScratch[i] = 0;
    }
}

static void BlitSegmentToIndependent(const u8 *src, u32 dstTileX, u32 dstTileY,
                                     u32 tilesW, u32 tilesH)
{
    for (u32 tileY = 0; tileY < tilesH; ++tileY) {
        for (u32 tileX = 0; tileX < tilesW; ++tileX) {
            const u8 *tile = src + ((tileY * tilesW + tileX) << 5);
            const u32 dstPixelYBase = (dstTileY + tileY) << 3;
            const u32 dstByteX = (dstTileX + tileX) << 2;
            for (u32 y = 0; y < 8u; ++y) {
                u8 *row = sIndepTextureScratch +
                    ((dstPixelYBase + y) * W2U_INDEP_TEX_STRIDE_BYTES) + dstByteX;
                const u8 *srcRow = tile + (y << 2);
                row[0] = (u8)((srcRow[0] << 4) | (srcRow[0] >> 4));
                row[1] = (u8)((srcRow[1] << 4) | (srcRow[1] >> 4));
                row[2] = (u8)((srcRow[2] << 4) | (srcRow[2] >> 4));
                row[3] = (u8)((srcRow[3] << 4) | (srcRow[3] >> 4));
            }
        }
    }
}

static b32 StageIndependentTexture(Actor *actor, u16 frame)
{
    if (!ReadFrame(actor, frame)) return false;
    ClearIndependentScratch();
    BlitSegmentToIndependent(sFrameScratch + 0x0000u, 2u, 2u, 8u, 8u);
    BlitSegmentToIndependent(sFrameScratch + 0x0800u, 10u, 2u, 4u, 8u);
    BlitSegmentToIndependent(sFrameScratch + 0x0c00u, 2u, 10u, 8u, 4u);
    BlitSegmentToIndependent(sFrameScratch + 0x1000u, 10u, 10u, 4u, 4u);
    return true;
}

static void SetTextureBanksLcdc(u8 *a, u8 *b, u8 *c, u8 *d)
{
    *a = *W2U_VRAMCNT_A;
    *b = *W2U_VRAMCNT_B;
    *c = *W2U_VRAMCNT_C;
    *d = *W2U_VRAMCNT_D;
    *W2U_VRAMCNT_A = W2U_VRAM_LCDC_ENABLE;
    *W2U_VRAMCNT_B = W2U_VRAM_LCDC_ENABLE;
    *W2U_VRAMCNT_C = W2U_VRAM_LCDC_ENABLE;
    *W2U_VRAMCNT_D = W2U_VRAM_LCDC_ENABLE;
}

static void RestoreTextureBanks(u8 a, u8 b, u8 c, u8 d)
{
    *W2U_VRAMCNT_A = a;
    *W2U_VRAMCNT_B = b;
    *W2U_VRAMCNT_C = c;
    *W2U_VRAMCNT_D = d;
}

static void SetTexturePaletteBanksLcdc(u8 *e, u8 *f, u8 *g)
{
    *e = *W2U_VRAMCNT_E;
    *f = *W2U_VRAMCNT_F;
    *g = *W2U_VRAMCNT_G;
    *W2U_VRAMCNT_E = W2U_VRAM_LCDC_ENABLE;
    *W2U_VRAMCNT_F = W2U_VRAM_LCDC_ENABLE;
    *W2U_VRAMCNT_G = W2U_VRAM_LCDC_ENABLE;
}

static void RestoreTexturePaletteBanks(u8 e, u8 f, u8 g)
{
    *W2U_VRAMCNT_E = e;
    *W2U_VRAMCNT_F = f;
    *W2U_VRAMCNT_G = g;
}

static s32 Clamp5(s32 value)
{
    if (value < 0) return 0;
    if (value > 31) return 31;
    return value;
}

static u16 FadeColor(u16 color, u16 target, s16 rate)
{
    if (rate >= 31) return target;
    if (rate <= 0) return color;
    const s32 rs = (s32)(color & 0x1fu);
    const s32 gs = (s32)((color >> 5) & 0x1fu);
    const s32 bs = (s32)((color >> 10) & 0x1fu);
    const s32 re = (s32)(target & 0x1fu);
    const s32 ge = (s32)((target >> 5) & 0x1fu);
    const s32 be = (s32)((target >> 10) & 0x1fu);
    const s32 r = Clamp5(rs + (((re - rs) * rate) >> 5));
    const s32 g = Clamp5(gs + (((ge - gs) * rate) >> 5));
    const s32 b = Clamp5(bs + (((be - bs) * rate) >> 5));
    return (u16)(r | (g << 5) | (b << 10));
}

static void BuildFadedPalette(const Actor *actor, void *pokeWk, u16 *out)
{
    out[0] = actor->asset.palette[0];
    const s16 rate = *(s16 *)((u8 *)pokeWk + W2U_INDEP_POKE_PAL_RATE_OFFSET);
    const u16 target = *(u16 *)((u8 *)pokeWk + W2U_INDEP_POKE_PAL_COLOR_OFFSET);
    for (u32 i = 1u; i < 16u; ++i) {
        out[i] = FadeColor(actor->asset.palette[i], target, rate);
    }
}

static void PatchIndependentRawPalette(const Actor *actor, void *pokeWk)
{
    if (!actor->pwan || !IsLikelyMainRamPointer(pokeWk)) {
        return;
    }
    u16 pal[16];
    BuildFadedPalette(actor, pokeWk, pal);
    u16 *palCurr = (u16 *)((u8 *)pokeWk + W2U_INDEP_POKE_PAL_CURR_OFFSET);
    for (u32 i = 0; i < 16u; ++i) {
        palCurr[i] = pal[i];
    }

    void *paletteData = *(void **)((u8 *)pokeWk + W2U_INDEP_POKE_PAL_PTR_OFFSET);
    if (!IsLikelyMainRamPointer(paletteData)) {
        return;
    }
    const u32 size = *(u32 *)((u8 *)paletteData + W2U_NNS_G2D_PALETTE_SZ_BYTE_OFFSET);
    u16 *raw = *(u16 **)((u8 *)paletteData + W2U_NNS_G2D_PALETTE_RAW_DATA_OFFSET);
    if (!IsLikelyMainRamPointer(raw) || size < 32u || size > 0x400u) {
        return;
    }
    for (u32 i = 0; i < 16u; ++i) {
        raw[i] = actor->asset.palette[i];
    }
}

static void PatchIndependentRawTexture(void *pokeWk)
{
    if (!IsLikelyMainRamPointer(pokeWk)) {
        return;
    }

    void *bmpData = *(void **)((u8 *)pokeWk + W2U_INDEP_POKE_BMP_PTR_OFFSET);
    if (IsLikelyMainRamPointer(bmpData)) {
        u8 *bmpRaw = *(u8 **)((u8 *)bmpData + W2U_GFL_BMP_CHAR_PTR_OFFSET);
        if (IsLikelyMainRamPointer(bmpRaw)) {
            for (u32 i = 0; i < W2U_INDEP_TEX_BYTES; ++i) {
                bmpRaw[i] = sIndepTextureScratch[i];
            }
        }
    }

}

static b32 UploadIndependent(Actor *actor, void *pokeWk)
{
    if (!actor->pwan || !IsLikelyMainRamPointer(pokeWk)) {
        return false;
    }
    const u16 frame = FrameForTick(&actor->asset, actor->tick);
    if (!StageIndependentTexture(actor, frame)) {
        return false;
    }

    const u32 texAdr = *(u32 *)((u8 *)pokeWk + W2U_INDEP_POKE_TEX_ADR_OFFSET);
    const u32 palAdr = *(u32 *)((u8 *)pokeWk + W2U_INDEP_POKE_PAL_ADR_OFFSET);
    u16 pal[16];
    BuildFadedPalette(actor, pokeWk, pal);
    PatchIndependentRawTexture(pokeWk);

    WaitForSafeVramUploadTime();
    u8 a, b, c, d;
    SetTextureBanksLcdc(&a, &b, &c, &d);
    volatile u16 *texDst = W2U_LCDC_TEX_VRAM + (texAdr >> 1);
    const u16 *texSrc = (const u16 *)sIndepTextureScratch;
    for (u32 i = 0; i < (W2U_INDEP_TEX_BYTES >> 1); ++i) {
        texDst[i] = texSrc[i];
    }
    RestoreTextureBanks(a, b, c, d);

    u8 e, f, g;
    SetTexturePaletteBanksLcdc(&e, &f, &g);
    volatile u16 *palDst = W2U_LCDC_TEX_PLTT + (palAdr >> 1);
    for (u32 i = 0; i < 16u; ++i) {
        palDst[i] = pal[i];
    }
    RestoreTexturePaletteBanks(e, f, g);

    actor->copiedIndepFrame = frame;
    W2U_Evolution_Profile.independentUploadCalls =
        W2U_Evolution_Profile.independentUploadCalls + 1u;
    return true;
}

static void UploadIndependentActors(void *work)
{
    if (!IsLikelyMainRamPointer(work)) {
        W2U_Evolution_Profile.skipReason = EVO_SKIP_BAD_INDEPENDENT;
        return;
    }
    void *mgr = *(void **)((u8 *)work + W2U_EVO_WORK_INDEPENDENT_MGR_OFFSET);
    W2U_Evolution_Profile.lastWork = (u32)work;
    W2U_Evolution_Profile.lastIndependentMgr = (u32)mgr;
    if (!IsLikelyMainRamPointer(mgr)) {
        W2U_Evolution_Profile.skipReason = EVO_SKIP_BAD_INDEPENDENT;
        return;
    }
    void *beforeWk = *(void **)((u8 *)mgr + W2U_INDEP_MGR_POKE0_OFFSET);
    void *afterWk = *(void **)((u8 *)mgr + W2U_INDEP_MGR_POKE1_OFFSET);
    W2U_Evolution_Profile.lastPokeWork[0] = (u32)beforeWk;
    W2U_Evolution_Profile.lastPokeWork[1] = (u32)afterWk;
    if (IsLikelyMainRamPointer(beforeWk)) {
        W2U_Evolution_Profile.lastTexAdr[0] =
            *(u32 *)((u8 *)beforeWk + W2U_INDEP_POKE_TEX_ADR_OFFSET);
        W2U_Evolution_Profile.lastPalAdr[0] =
            *(u32 *)((u8 *)beforeWk + W2U_INDEP_POKE_PAL_ADR_OFFSET);
    }
    if (IsLikelyMainRamPointer(afterWk)) {
        W2U_Evolution_Profile.lastTexAdr[1] =
            *(u32 *)((u8 *)afterWk + W2U_INDEP_POKE_TEX_ADR_OFFSET);
        W2U_Evolution_Profile.lastPalAdr[1] =
            *(u32 *)((u8 *)afterWk + W2U_INDEP_POKE_PAL_ADR_OFFSET);
    }

    PatchIndependentRawPalette(&sState.before, beforeWk);
    PatchIndependentRawPalette(&sState.after, afterWk);

    if (sState.before.pwan && UploadIndependent(&sState.before, beforeWk)) {
        W2U_Evolution_Profile.beforeCopiedIndependentFrame =
            sState.before.copiedIndepFrame;
        AdvanceTick(&sState.before);
    }
    if (sState.after.pwan && UploadIndependent(&sState.after, afterWk)) {
        W2U_Evolution_Profile.afterCopiedIndependentFrame =
            sState.after.copiedIndepFrame;
        AdvanceTick(&sState.after);
    }
}

static void PatchIndependentRawPalettes(void *work)
{
    if (!sState.active || !IsLikelyMainRamPointer(work)) {
        return;
    }
    void *mgr = *(void **)((u8 *)work + W2U_EVO_WORK_INDEPENDENT_MGR_OFFSET);
    if (!IsLikelyMainRamPointer(mgr)) {
        return;
    }
    void *beforeWk = *(void **)((u8 *)mgr + W2U_INDEP_MGR_POKE0_OFFSET);
    void *afterWk = *(void **)((u8 *)mgr + W2U_INDEP_MGR_POKE1_OFFSET);
    PatchIndependentRawPalette(&sState.before, beforeWk);
    PatchIndependentRawPalette(&sState.after, afterWk);
}

static Actor *VisibleMcssActor()
{
    if (sState.phase == PHASE_POST_MCSS) {
        return &sState.after;
    }
    return &sState.before;
}

static void DrawVisibleObjActor(Actor *actor)
{
    if (!actor || !actor->pwan) {
        HideOam();
        return;
    }
    PrepareObjDisplay();
    CopyObjPalette(actor);
    const u16 frame = FrameForTick(&actor->asset, actor->tick);
    if (sState.copiedObjAsset != actor->assetId || sState.copiedObjFrame != frame) {
        if (!CopyObjFrame(actor, frame)) {
            HideOam();
            return;
        }
    } else if (!CopyObjFrame(actor, frame)) {
        HideOam();
        return;
    }
    DrawObjFrame();
    AdvanceTick(actor);
}

static void DrawReadyObjActor()
{
    if (!sState.readyToDrawObj) {
        return;
    }
    sState.readyToDrawObj = false;
    DrawVisibleObjActor(VisibleMcssActor());
}

} // namespace evolution_anim
} // namespace w2u

extern "C" void *W2U_Evolution_AddPokeMcssLoop(void *system, const void *pp, int dir,
                                                s32 x, s32 y, s32 z)
{
    void *mcss = w2u::evolution_anim::NativeAddPokeSprite_Fn(system, pp, dir, x, y, z);
    w2u::evolution_anim::W2U_Evolution_Profile.addLoopCalls =
        w2u::evolution_anim::W2U_Evolution_Profile.addLoopCalls + 1u;

    if (w2u::evolution_anim::sState.addLoopIndex == 0) {
        w2u::evolution_anim::ClearState();
        w2u::evolution_anim::RegisterActor(&w2u::evolution_anim::sState.before,
                                           system, mcss, pp);
        w2u::evolution_anim::sState.addLoopIndex = 1u;
    } else {
        w2u::evolution_anim::RegisterActor(&w2u::evolution_anim::sState.after,
                                           system, mcss, pp);
        w2u::evolution_anim::sState.addLoopIndex = 0u;
        w2u::evolution_anim::UpdateActiveFlag();
    }

    w2u::evolution_anim::W2U_Evolution_Profile.lastSystem = (u32)system;
    w2u::evolution_anim::W2U_Evolution_Profile.lastMcss = (u32)mcss;
    return mcss;
}

extern "C" void *W2U_Evolution_AddPokeMcssSingle(void *system, const void *pp, int dir,
                                                  s32 x, s32 y, s32 z)
{
    void *mcss = w2u::evolution_anim::NativeAddPokeSprite_Fn(system, pp, dir, x, y, z);
    w2u::evolution_anim::W2U_Evolution_Profile.addSingleCalls =
        w2u::evolution_anim::W2U_Evolution_Profile.addSingleCalls + 1u;
    w2u::evolution_anim::ClearState();
    w2u::evolution_anim::RegisterActor(&w2u::evolution_anim::sState.after,
                                       system, mcss, pp);
    w2u::evolution_anim::UpdateActiveFlag();
    w2u::evolution_anim::sState.phase = w2u::evolution_anim::PHASE_POST_MCSS;
    w2u::evolution_anim::W2U_Evolution_Profile.lastSystem = (u32)system;
    w2u::evolution_anim::W2U_Evolution_Profile.lastMcss = (u32)mcss;
    return mcss;
}

extern "C" void W2U_Evolution_MainMcss(void *system)
{
    w2u::evolution_anim::RestoreNativeVramMapping();
    w2u::evolution_anim::MCSS_Main_Fn(system);
}

extern "C" void W2U_Evolution_MainIndependent(void *work)
{
    w2u::evolution_anim::RestoreNativeVramMapping();
    w2u::evolution_anim::PatchIndependentRawPalettes(work);
    w2u::evolution_anim::EvoStaticRendererMain_Fn(work);
    w2u::evolution_anim::PatchIndependentRawPalettes(work);
}

extern "C" void W2U_Evolution_DrawMcss(void *system)
{
    w2u::evolution_anim::W2U_Evolution_Profile.drawMcssCalls =
        w2u::evolution_anim::W2U_Evolution_Profile.drawMcssCalls + 1u;
    if (!w2u::evolution_anim::sState.active) {
        w2u::evolution_anim::W2U_Evolution_Profile.skipReason =
            w2u::evolution_anim::EVO_SKIP_INACTIVE;
        w2u::evolution_anim::sState.readyToDrawObj = false;
        w2u::evolution_anim::HideOam();
        w2u::evolution_anim::NativeMcssDraw(system);
        return;
    }
    if (w2u::evolution_anim::sState.after.system != system &&
        w2u::evolution_anim::sState.before.system != system) {
        w2u::evolution_anim::W2U_Evolution_Profile.skipReason =
            w2u::evolution_anim::EVO_SKIP_WRONG_SYSTEM;
        w2u::evolution_anim::sState.readyToDrawObj = false;
        w2u::evolution_anim::HideOam();
        w2u::evolution_anim::NativeMcssDraw(system);
        return;
    }

    if (w2u::evolution_anim::sState.sawIndependent) {
        w2u::evolution_anim::sState.phase = w2u::evolution_anim::PHASE_POST_MCSS;
    }
    w2u::evolution_anim::W2U_Evolution_Profile.phase = w2u::evolution_anim::sState.phase;
    w2u::evolution_anim::W2U_Evolution_Profile.skipReason =
        w2u::evolution_anim::EVO_SKIP_NONE;
    w2u::evolution_anim::sState.mcssDrawFrames =
        w2u::evolution_anim::sState.mcssDrawFrames + 1u;

    w2u::evolution_anim::Actor *actor = w2u::evolution_anim::VisibleMcssActor();
    if (actor->pwan) {
        w2u::evolution_anim::HideNativeMcss(actor->mcss);
        w2u::evolution_anim::sState.readyToDrawObj = true;
    } else {
        w2u::evolution_anim::sState.readyToDrawObj = false;
        w2u::evolution_anim::HideOam();
    }
    w2u::evolution_anim::NativeMcssDraw(system);
}

extern "C" void W2U_Evolution_DrawIndependent(void *work)
{
    w2u::evolution_anim::W2U_Evolution_Profile.drawIndependentCalls =
        w2u::evolution_anim::W2U_Evolution_Profile.drawIndependentCalls + 1u;
    w2u::evolution_anim::sState.readyToDrawObj = false;
    w2u::evolution_anim::HideOam();
    if (w2u::evolution_anim::sState.active) {
        w2u::evolution_anim::sState.phase = w2u::evolution_anim::PHASE_INDEPENDENT;
        w2u::evolution_anim::sState.sawIndependent = true;
        w2u::evolution_anim::W2U_Evolution_Profile.phase =
            w2u::evolution_anim::sState.phase;
        w2u::evolution_anim::RestoreNativeVramMapping();
        w2u::evolution_anim::UploadIndependentActors(work);
    }
    w2u::evolution_anim::RestoreNativeVramMapping();
    w2u::evolution_anim::EvoStaticRendererDraw_Fn(work);
    w2u::evolution_anim::W2U_Evolution_Profile.nativeIndependentDrawCalls =
        w2u::evolution_anim::W2U_Evolution_Profile.nativeIndependentDrawCalls + 1u;
}

extern "C" void W2U_Evolution_AfterGraphicEnd(void *work)
{
    w2u::evolution_anim::EvoGraphicEndDraw_Fn(work);
    w2u::evolution_anim::DrawReadyObjActor();
}

extern "C" void W2U_Evolution_Del(void *system, void *mcss)
{
    w2u::evolution_anim::W2U_Evolution_Profile.deleteCalls =
        w2u::evolution_anim::W2U_Evolution_Profile.deleteCalls + 1u;
    if (w2u::evolution_anim::sState.before.mcss == mcss) {
        w2u::evolution_anim::ClearActor(&w2u::evolution_anim::sState.before);
    }
    if (w2u::evolution_anim::sState.after.mcss == mcss) {
        w2u::evolution_anim::ClearActor(&w2u::evolution_anim::sState.after);
    }
    if (!w2u::evolution_anim::sState.before.active &&
        !w2u::evolution_anim::sState.after.active) {
        w2u::evolution_anim::HideOam();
        w2u::evolution_anim::ClearState();
    }
    w2u::evolution_anim::MCSS_Del_Fn(system, mcss);
}
