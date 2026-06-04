#include "Species.h"
#include "nds/fs.h"
#include "pwan_types.h"
#include "w2u_pwan_config.h"

#define W2U_PWAN_MAGIC 0x4E415750u
#define W2U_FRAME_BYTES 0x1200u
#define W2U_MCSS_TEX_WIDTH 256u
#define W2U_MCSS_TEX_HEIGHT 128u
#define W2U_MCSS_TEX_STRIDE_BYTES (W2U_MCSS_TEX_WIDTH >> 1)
#define W2U_MCSS_TEX_ROW_HALFWORDS (W2U_MCSS_TEX_STRIDE_BYTES >> 1)
#define W2U_VISIBLE_TEX_WIDTH 96u
#define W2U_VISIBLE_TEX_HEIGHT 96u
#define W2U_VISIBLE_TEX_ROW_BYTES (W2U_VISIBLE_TEX_WIDTH >> 1)
#define W2U_VISIBLE_TEX_ROW_HALFWORDS (W2U_VISIBLE_TEX_ROW_BYTES >> 1)
#define W2U_VISIBLE_TEX_BYTES (W2U_VISIBLE_TEX_ROW_BYTES * W2U_VISIBLE_TEX_HEIGHT)
#define W2U_STAGING_TEX_BYTES (W2U_MCSS_TEX_STRIDE_BYTES * W2U_MCSS_TEX_HEIGHT)
#define W2U_MCSS_TEX_SLOT_BYTES 0x4000u
#define W2U_MCSS_TEX_SLOT_COUNT 8u
#define W2U_MCSS_PLTT_BASE 0x1000u
#define W2U_PWAN_PLTT_BASE 0x1800u
#define W2U_MCSS_PLTT_SLOT_BYTES 0x20u
#define W2U_MCSS_DEFAULT_TEX_BASE 0x00000u
#define W2U_MCSS_PARTICLE_TEX_BASE 0x30000u
#define W2U_NB_BASELINE_RAISE_PX 5u
#define W2U_MCSS_FLAGS_OFFSET 0x140u
#define W2U_MCSS_FLAGS_LOAD_SHIFT 13u
#define W2U_MCSS_INDEX_OFFSET 0x148u
#define W2U_MCSS_IMAGE_PROXY_VRAM_OFFSET 0x9cu
#define W2U_MCSS_PALETTE_PROXY_VRAM_OFFSET 0xc8u
#define W2U_MCSS_BASE_PLTT_DATA_OFFSET 0xd4u
#define W2U_MCSS_FADE_PLTT_DATA_OFFSET 0xd8u
#define W2U_MCSS_PLTT_DATA_SIZE_OFFSET 0xdcu
#define W2U_LCDC_TEX_VRAM ((volatile u16 *)0x06800000)
#define W2U_LCDC_TEX_PLTT ((volatile u16 *)0x06890000)
#define W2U_VRAMCNT_A ((volatile u8 *)0x04000240)
#define W2U_VRAMCNT_B ((volatile u8 *)0x04000241)
#define W2U_VRAMCNT_C ((volatile u8 *)0x04000242)
#define W2U_VRAMCNT_D ((volatile u8 *)0x04000243)
#define W2U_VRAMCNT_E ((volatile u8 *)0x04000244)
#define W2U_VRAMCNT_F ((volatile u8 *)0x04000245)
#define W2U_VRAMCNT_G ((volatile u8 *)0x04000246)
#define W2U_VRAM_LCDC_ENABLE 0x80u
#define W2U_REG_VCOUNT ((volatile u16 *)0x04000006)
#define W2U_MCSS_VCOUNT_LOW 192u
#define W2U_MCSS_VCOUNT_HIGH 200u
#define W2U_MAIN_RAM_START 0x02000000u
#define W2U_MAIN_RAM_END 0x02400000u

namespace w2u {
namespace nonbattle_mcss {

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
#define W2U_NB_ACTOR_COUNT 8u
#define W2U_NB_PENDING_COUNT 4u

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
    b32 textureDirty;
    b32 paletteDirty;
    b32 freezeFirstFrame;
    void *system;
    void *mcss;
    u16 species;
    u16 copiedFrame;
    u16 pendingFrame;
    u32 tick;
    u32 texBase;
    AssetId assetId;
    Asset asset;
};

struct PendingMaw {
    b32 active;
    void *maw;
    u16 species;
    u8 dir;
    b32 freezeFirstFrame;
    u32 texBase;
};

struct State {
    Actor actor[W2U_NB_ACTOR_COUNT];
    PendingMaw pending[W2U_NB_PENDING_COUNT];
    u8 nextUploadActor;
};

struct NonBattleProfile {
    u32 magic;
    u32 version;
    u32 structSize;
    u32 registerCalls;
    u32 drawCalls;
    u32 deleteCalls;
    u32 uploadCalls;
    u32 uploadActors;
    u32 loadFailCount;
    u32 lastSystem;
    u32 lastMcss;
    u32 lastSpecies;
    u32 lastAsset;
    u32 lastTexBase;
    u32 lastMcssIndex;
    u32 lastActiveMask;
    u32 lastDirtyMask;
    u32 waitCalls;
    u32 maxWaitSpinIterations;
    u32 lastVcountBeforeWait;
    u32 lastVcountAfterWait;
    u32 paletteCpuCopyFailCount;
};

extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
volatile NonBattleProfile W2U_NonBattleMcss_Profile = {
    0x46424E50u,
    3u,
    sizeof(NonBattleProfile),
};
#pragma GCC diagnostic pop
}

static State sState;
static char sAssetPath[W2U_PWAN_PATH_BYTES];
static u8 sFrameScratch[W2U_FRAME_BYTES];
static u8 sTextureScratch[W2U_STAGING_TEX_BYTES];

typedef u32 (*PpGetFn)(const void *pp, int id, void *buf);
typedef void (*BuildSpriteParamsFn)(int monsNo, int formNo, int sex, int rare, b32 egg,
                               void *maw, int dir);
typedef void (*BuildSpriteParamsFromPpFn)(const void *pp, void *maw, int dir);
typedef void *(*AddPokeMcssFn)(void *system, const void *pp, int dir, s32 x, s32 y, s32 z);
typedef void *(*McssAddFn)(void *system, s32 x, s32 y, s32 z, const void *maw);
typedef void (*McssDrawFn)(void *system);
typedef void (*McssDelFn)(void *system, void *mcss);

static PpGetFn const PP_Get_Fn = (PpGetFn)0x0201CD25u;
static BuildSpriteParamsFn const NativeBuildSpriteParams_Fn = (BuildSpriteParamsFn)0x0201C071u;
static BuildSpriteParamsFromPpFn const NativeBuildSpriteParamsFromPp_Fn = (BuildSpriteParamsFromPpFn)0x0201C009u;
static AddPokeMcssFn const NativeAddPokeSprite_Fn = (AddPokeMcssFn)0x0201C179u;
static McssAddFn const MCSS_Add_Fn = (McssAddFn)0x0201A8D5u;
static McssDrawFn const MCSS_Draw_Fn = (McssDrawFn)0x02019C39u;
static McssDelFn const MCSS_Del_Fn = (McssDelFn)0x0201AAADu;

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

static void ClearTextureScratch(void)
{
    volatile u8 *dst = sTextureScratch;
    for (u32 i = 0; i < W2U_STAGING_TEX_BYTES; ++i) {
        dst[i] = 0;
    }
}

static void BlitTileSegmentToTexture(u8 *dst, const u8 *src, u32 dstX, s32 dstY,
                                     u32 tilesW, u32 tilesH)
{
    for (u32 tileY = 0; tileY < tilesH; ++tileY) {
        for (u32 tileX = 0; tileX < tilesW; ++tileX) {
            const u8 *tile = src + ((tileY * tilesW + tileX) * 32u);
            for (u32 y = 0; y < 8; ++y) {
                const s32 rowY = dstY + (s32)(tileY * 8u + y);
                if (rowY < 0 || rowY >= (s32)W2U_VISIBLE_TEX_HEIGHT) continue;
                u8 *row = dst + (u32)rowY * W2U_MCSS_TEX_STRIDE_BYTES +
                          ((dstX + tileX * 8u) >> 1);
                const u8 *srcRow = tile + y * 4u;
                row[0] = srcRow[0];
                row[1] = srcRow[1];
                row[2] = srcRow[2];
                row[3] = srcRow[3];
            }
        }
    }
}

static void ConvertFrameToTexture(void)
{
    const s32 dstY = -(s32)W2U_NB_BASELINE_RAISE_PX;
    ClearTextureScratch();
    BlitTileSegmentToTexture(sTextureScratch, sFrameScratch + 0x0000u, 0, dstY, 8, 8);
    BlitTileSegmentToTexture(sTextureScratch, sFrameScratch + 0x0800u, 64, dstY, 4, 8);
    BlitTileSegmentToTexture(sTextureScratch, sFrameScratch + 0x0c00u, 0,
                             dstY + 64, 8, 4);
    BlitTileSegmentToTexture(sTextureScratch, sFrameScratch + 0x1000u, 64,
                             dstY + 64, 4, 4);
}

static b32 StageFrameTexture(Actor *actor, u16 frame)
{
    const AssetId assetId = actor->assetId;
    if (assetId >= W2U_PWAN_ASSET_COUNT) {
        return false;
    }
    const u32 offset = actor->asset.header.frameOffset + frame * actor->asset.header.frameBytes;
    if (!ReadAssetRange(&actor->asset, offset, sFrameScratch, W2U_FRAME_BYTES)) {
        return false;
    }
    ConvertFrameToTexture();
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

static b32 IsSafeVramUploadTime()
{
    const u16 vcount = *W2U_REG_VCOUNT;
    return vcount >= W2U_MCSS_VCOUNT_LOW && vcount <= W2U_MCSS_VCOUNT_HIGH;
}

static void WaitForSafeVramUploadTime()
{
    u32 spins = 0;
    W2U_NonBattleMcss_Profile.waitCalls = W2U_NonBattleMcss_Profile.waitCalls + 1u;
    W2U_NonBattleMcss_Profile.lastVcountBeforeWait = *W2U_REG_VCOUNT;
    while (!IsSafeVramUploadTime()) {
        spins = spins + 1u;
    }
    if (spins > W2U_NonBattleMcss_Profile.maxWaitSpinIterations) {
        W2U_NonBattleMcss_Profile.maxWaitSpinIterations = spins;
    }
    W2U_NonBattleMcss_Profile.lastVcountAfterWait = *W2U_REG_VCOUNT;
}

static s32 GetMcssIndex(void *mcss)
{
    if (!IsLikelyMainRamPointer(mcss)) {
        return -1;
    }
    return *(s32 *)((u8 *)mcss + W2U_MCSS_INDEX_OFFSET);
}

static u32 GetMcssFlags(void *mcss)
{
    if (!IsLikelyMainRamPointer(mcss)) {
        return 0;
    }
    return *(u32 *)((u8 *)mcss + W2U_MCSS_FLAGS_OFFSET);
}

static b32 McssResourcesReady(void *mcss)
{
    return ((GetMcssFlags(mcss) >> W2U_MCSS_FLAGS_LOAD_SHIFT) & 1u) != 0;
}

static b32 IsLikelyTextureVramOffset(u32 offset)
{
    return offset < 0x80000u;
}

static u32 GetMcssTextureBase(void *mcss, s32 mcssIndex, u32 fallbackTexBase)
{
    if (IsLikelyMainRamPointer(mcss)) {
        const u32 proxyBase = *(u32 *)((u8 *)mcss + W2U_MCSS_IMAGE_PROXY_VRAM_OFFSET);
        if (IsLikelyTextureVramOffset(proxyBase)) {
            return proxyBase;
        }
    }
    if (mcssIndex < 0 || (u32)mcssIndex >= W2U_MCSS_TEX_SLOT_COUNT) {
        return fallbackTexBase;
    }
    return fallbackTexBase + W2U_MCSS_TEX_SLOT_BYTES * (u32)mcssIndex;
}

static u32 GetPwanPaletteBase(s32 mcssIndex)
{
    if (mcssIndex < 0 || (u32)mcssIndex >= W2U_MCSS_TEX_SLOT_COUNT) {
        return W2U_PWAN_PLTT_BASE;
    }
    return W2U_PWAN_PLTT_BASE + W2U_MCSS_PLTT_SLOT_BYTES * (u32)mcssIndex;
}

static void SetMcssPaletteBase(void *mcss, u32 paletteBase)
{
    if (IsLikelyMainRamPointer(mcss)) {
        *(u32 *)((u8 *)mcss + W2U_MCSS_PALETTE_PROXY_VRAM_OFFSET) = paletteBase;
    }
}

static void CopyPaletteToLiveMcss(Actor *actor)
{
    void *mcss = actor->mcss;
    if (!IsLikelyMainRamPointer(mcss)) {
        W2U_NonBattleMcss_Profile.paletteCpuCopyFailCount =
            W2U_NonBattleMcss_Profile.paletteCpuCopyFailCount + 1u;
        return;
    }

    u16 *base = *(u16 **)((u8 *)mcss + W2U_MCSS_BASE_PLTT_DATA_OFFSET);
    u16 *fade = *(u16 **)((u8 *)mcss + W2U_MCSS_FADE_PLTT_DATA_OFFSET);
    const u32 size = *(u32 *)((u8 *)mcss + W2U_MCSS_PLTT_DATA_SIZE_OFFSET);
    if (!IsLikelyMainRamPointer(base) || !IsLikelyMainRamPointer(fade) ||
        size < W2U_MCSS_PLTT_SLOT_BYTES || size > 0x200u) {
        W2U_NonBattleMcss_Profile.paletteCpuCopyFailCount =
            W2U_NonBattleMcss_Profile.paletteCpuCopyFailCount + 1u;
        return;
    }

    for (u32 i = 0; i < 16; ++i) {
        base[i] = actor->asset.palette[i];
        fade[i] = actor->asset.palette[i];
    }
}

static void UploadPalette(Actor *actor, u32 paletteBase)
{
    u8 e, f, g;
    SetTexturePaletteBanksLcdc(&e, &f, &g);
    volatile u16 *dst = W2U_LCDC_TEX_PLTT + (paletteBase >> 1);
    for (u32 i = 0; i < 16; ++i) {
        dst[i] = actor->asset.palette[i];
    }
    RestoreTexturePaletteBanks(e, f, g);
    actor->paletteDirty = false;
}

static void UploadTexture(Actor *actor, u32 textureBase)
{
    u8 a, b, c, d;
    SetTextureBanksLcdc(&a, &b, &c, &d);
    volatile u16 *dst = W2U_LCDC_TEX_VRAM + (textureBase >> 1);
    for (u32 y = 0; y < W2U_MCSS_TEX_HEIGHT; ++y) {
        volatile u16 *dstRow = dst + ((y * W2U_MCSS_TEX_STRIDE_BYTES) >> 1);
        const u16 *srcRow = (const u16 *)(sTextureScratch + y * W2U_MCSS_TEX_STRIDE_BYTES);
        for (u32 x = 0; x < W2U_MCSS_TEX_ROW_HALFWORDS; ++x) {
            dstRow[x] = srcRow[x];
        }
    }
    RestoreTextureBanks(a, b, c, d);
    actor->textureDirty = false;
    W2U_NonBattleMcss_Profile.uploadActors = W2U_NonBattleMcss_Profile.uploadActors + 1u;
}

static void ClearActor(Actor *actor)
{
    CloseAssetFile(&actor->asset);
    actor->asset.loaded = false;
    actor->asset.assetId = ASSET_NONE;
    actor->active = false;
    actor->textureDirty = false;
    actor->paletteDirty = false;
    actor->freezeFirstFrame = false;
    actor->system = 0;
    actor->mcss = 0;
    actor->species = SPECIES_NONE;
    actor->copiedFrame = 0xffffu;
    actor->pendingFrame = 0xffffu;
    actor->tick = 0;
    actor->texBase = W2U_MCSS_DEFAULT_TEX_BASE;
    actor->assetId = ASSET_NONE;
}

static Actor *FindActor(void *mcss)
{
    for (u32 i = 0; i < W2U_NB_ACTOR_COUNT; ++i) {
        if (sState.actor[i].active && sState.actor[i].mcss == mcss) {
            return &sState.actor[i];
        }
    }
    return 0;
}

static Actor *AllocActor(void *mcss)
{
    Actor *actor = FindActor(mcss);
    if (actor) return actor;
    for (u32 i = 0; i < W2U_NB_ACTOR_COUNT; ++i) {
        if (!sState.actor[i].active) {
            return &sState.actor[i];
        }
    }
    return &sState.actor[0];
}

static void RemoveActor(void *mcss)
{
    Actor *actor = FindActor(mcss);
    if (actor) {
        ClearActor(actor);
    }
}

static void RegisterPending(void *maw, u16 species, u8 dir, b32 freezeFirstFrame,
                            u32 texBase)
{
    if (!maw || species == SPECIES_NONE) {
        return;
    }
    for (u32 i = 0; i < W2U_NB_PENDING_COUNT; ++i) {
        if (!sState.pending[i].active || sState.pending[i].maw == maw) {
            sState.pending[i].active = true;
            sState.pending[i].maw = maw;
            sState.pending[i].species = species;
            sState.pending[i].dir = dir;
            sState.pending[i].freezeFirstFrame = freezeFirstFrame;
            sState.pending[i].texBase = texBase;
            return;
        }
    }
    sState.pending[0].active = true;
    sState.pending[0].maw = maw;
    sState.pending[0].species = species;
    sState.pending[0].dir = dir;
    sState.pending[0].freezeFirstFrame = freezeFirstFrame;
    sState.pending[0].texBase = texBase;
}

static b32 ConsumePending(void *maw, PendingMaw *out)
{
    for (u32 i = 0; i < W2U_NB_PENDING_COUNT; ++i) {
        if (sState.pending[i].active && sState.pending[i].maw == maw) {
            *out = sState.pending[i];
            sState.pending[i].active = false;
            return true;
        }
    }
    return false;
}

static u16 GetPpSpecies(const void *pp)
{
    if (!pp) return SPECIES_NONE;
    return (u16)PP_Get_Fn(pp, 5, 0);
}

static void RegisterActor(void *system, void *mcss, u16 species, u8 dir,
                          b32 freezeFirstFrame, u32 texBase)
{
    W2U_NonBattleMcss_Profile.registerCalls = W2U_NonBattleMcss_Profile.registerCalls + 1u;
    W2U_NonBattleMcss_Profile.lastSystem = (u32)system;
    W2U_NonBattleMcss_Profile.lastMcss = (u32)mcss;
    W2U_NonBattleMcss_Profile.lastSpecies = species;
    W2U_NonBattleMcss_Profile.lastTexBase = texBase;

    if (!system || !IsLikelyMainRamPointer(mcss) || species == SPECIES_NONE) {
        RemoveActor(mcss);
        return;
    }

    const b32 isFront = dir == 0;
    const AssetId assetId = GetAssetForSpeciesSide(species, isFront);
    W2U_NonBattleMcss_Profile.lastAsset = assetId;
    if (assetId >= W2U_PWAN_ASSET_COUNT) {
        RemoveActor(mcss);
        return;
    }

    Actor *actor = AllocActor(mcss);
    const b32 wasInactive = !actor->active;
    const b32 changed = actor->mcss != mcss || actor->species != species ||
                        actor->assetId != assetId || actor->texBase != texBase ||
                        actor->freezeFirstFrame != freezeFirstFrame;
    actor->active = true;
    actor->freezeFirstFrame = freezeFirstFrame;
    actor->system = system;
    actor->mcss = mcss;
    actor->species = species;
    actor->assetId = assetId;
    actor->texBase = texBase;

    if (!LoadAsset(actor, assetId)) {
        W2U_NonBattleMcss_Profile.loadFailCount = W2U_NonBattleMcss_Profile.loadFailCount + 1u;
        ClearActor(actor);
        return;
    }
    if (wasInactive || changed) {
        actor->tick = 0;
        actor->copiedFrame = 0xffffu;
        actor->pendingFrame = 0xffffu;
        actor->paletteDirty = true;
        actor->textureDirty = true;
    }
}

static void UpdateActor(Actor *actor)
{
    if (actor->freezeFirstFrame) {
        if (actor->copiedFrame != 0) {
            actor->pendingFrame = 0;
            actor->textureDirty = true;
        }
        actor->tick = 0;
        return;
    }

    const u32 totalTicks = actor->asset.header.totalTicks ? actor->asset.header.totalTicks : 1u;
    if (actor->tick >= totalTicks) {
        actor->tick = 0;
    }
    const u16 frame = FrameForTick(&actor->asset, actor->tick);
    if (frame != actor->copiedFrame) {
        actor->pendingFrame = frame;
        actor->textureDirty = true;
    }
    actor->tick = actor->tick + 1u;
    if (actor->tick >= totalTicks) {
        actor->tick = 0;
    }
}

static void UploadDirtyActors(void *system)
{
    W2U_NonBattleMcss_Profile.lastActiveMask = 0;
    W2U_NonBattleMcss_Profile.lastDirtyMask = 0;
    for (u32 i = 0; i < W2U_NB_ACTOR_COUNT; ++i) {
        Actor *actor = &sState.actor[i];
        if (!actor->active || actor->system != system) continue;
        W2U_NonBattleMcss_Profile.lastActiveMask |= (1u << i);
        if (!McssResourcesReady(actor->mcss)) {
            actor->textureDirty = true;
            actor->paletteDirty = true;
            W2U_NonBattleMcss_Profile.lastDirtyMask |= (1u << i);
            continue;
        }
        UpdateActor(actor);
        actor->paletteDirty = true;
        if (actor->textureDirty || actor->paletteDirty) {
            W2U_NonBattleMcss_Profile.lastDirtyMask |= (1u << i);
        }
    }

    u32 uploaded = 0;
    for (u32 attempt = 0; attempt < W2U_NB_ACTOR_COUNT && uploaded < 2u; ++attempt) {
        u32 actorIndex = sState.nextUploadActor + attempt;
        while (actorIndex >= W2U_NB_ACTOR_COUNT) {
            actorIndex -= W2U_NB_ACTOR_COUNT;
        }
        Actor *actor = &sState.actor[actorIndex];
        if (!actor->active || actor->system != system ||
            (!actor->textureDirty && !actor->paletteDirty)) {
            continue;
        }
        if (!McssResourcesReady(actor->mcss)) {
            continue;
        }
        const s32 mcssIndex = GetMcssIndex(actor->mcss);
        W2U_NonBattleMcss_Profile.lastMcssIndex = (u32)mcssIndex;
        if (mcssIndex < 0 || (u32)mcssIndex >= W2U_MCSS_TEX_SLOT_COUNT) {
            if (!IsLikelyMainRamPointer(actor->mcss)) {
                ClearActor(actor);
                continue;
            }
        }
        const u32 textureBase = GetMcssTextureBase(actor->mcss, mcssIndex, actor->texBase);
        const u32 paletteBase = GetPwanPaletteBase(mcssIndex);
        SetMcssPaletteBase(actor->mcss, paletteBase);
        const b32 uploadTexture = actor->textureDirty;
        const b32 stagedTexture = uploadTexture ?
            StageFrameTexture(actor, actor->pendingFrame) : true;
        if (stagedTexture) {
            WaitForSafeVramUploadTime();
            if (actor->paletteDirty || uploadTexture) {
                CopyPaletteToLiveMcss(actor);
                UploadPalette(actor, paletteBase);
            }
            if (uploadTexture) {
                UploadTexture(actor, textureBase);
                actor->copiedFrame = actor->pendingFrame;
            }
            uploaded = uploaded + 1u;
            W2U_NonBattleMcss_Profile.uploadCalls = W2U_NonBattleMcss_Profile.uploadCalls + 1u;
        } else {
            actor->textureDirty = false;
            actor->paletteDirty = false;
        }
        actorIndex = actorIndex + 1u;
        if (actorIndex >= W2U_NB_ACTOR_COUNT) {
            actorIndex = 0;
        }
        sState.nextUploadActor = (u8)actorIndex;
    }
}

} // namespace nonbattle_mcss
} // namespace w2u

extern "C" void W2U_NonBattle_BuildSpriteParams(int monsNo, int formNo, int sex, int rare,
                                           b32 egg, void *maw, int dir)
{
    w2u::nonbattle_mcss::NativeBuildSpriteParams_Fn(monsNo, formNo, sex, rare, egg, maw, dir);
    w2u::nonbattle_mcss::RegisterPending(maw, (u16)monsNo, (u8)dir,
                                         false, W2U_MCSS_DEFAULT_TEX_BASE);
}

extern "C" void W2U_NonBattle_BuildSpriteParamsFromPp(const void *pp, void *maw, int dir)
{
    w2u::nonbattle_mcss::NativeBuildSpriteParamsFromPp_Fn(pp, maw, dir);
    w2u::nonbattle_mcss::RegisterPending(maw, w2u::nonbattle_mcss::GetPpSpecies(pp),
                                         (u8)dir, true, W2U_MCSS_DEFAULT_TEX_BASE);
}

extern "C" void *W2U_NonBattle_Add(void *system, s32 x, s32 y, s32 z, const void *maw)
{
    void *mcss = w2u::nonbattle_mcss::MCSS_Add_Fn(system, x, y, z, maw);
    w2u::nonbattle_mcss::PendingMaw pending;
    if (w2u::nonbattle_mcss::ConsumePending((void *)maw, &pending)) {
        w2u::nonbattle_mcss::RegisterActor(system, mcss, pending.species, pending.dir,
                                           pending.freezeFirstFrame, pending.texBase);
    }
    return mcss;
}

extern "C" void *W2U_NonBattle_AddPokeMcss(void *system, const void *pp, int dir,
                                           s32 x, s32 y, s32 z)
{
    void *mcss = w2u::nonbattle_mcss::NativeAddPokeSprite_Fn(system, pp, dir, x, y, z);
    w2u::nonbattle_mcss::RegisterActor(system, mcss, w2u::nonbattle_mcss::GetPpSpecies(pp),
                                       (u8)dir, false, W2U_MCSS_PARTICLE_TEX_BASE);
    return mcss;
}

extern "C" void W2U_NonBattle_Draw(void *system)
{
    w2u::nonbattle_mcss::W2U_NonBattleMcss_Profile.drawCalls =
        w2u::nonbattle_mcss::W2U_NonBattleMcss_Profile.drawCalls + 1u;
    w2u::nonbattle_mcss::UploadDirtyActors(system);
    w2u::nonbattle_mcss::MCSS_Draw_Fn(system);
}

extern "C" void W2U_NonBattle_Del(void *system, void *mcss)
{
    w2u::nonbattle_mcss::W2U_NonBattleMcss_Profile.deleteCalls =
        w2u::nonbattle_mcss::W2U_NonBattleMcss_Profile.deleteCalls + 1u;
    w2u::nonbattle_mcss::RemoveActor(mcss);
    w2u::nonbattle_mcss::MCSS_Del_Fn(system, mcss);
}
