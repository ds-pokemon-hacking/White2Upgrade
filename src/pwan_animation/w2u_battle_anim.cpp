#include "Species.h"
#include "nds/fs.h"
#include "pwan_types.h"
#include "string.h"
#include "w2u_pwan_config.h"

#define W2U_PWAN_MAGIC 0x4E415750u
#define W2U_FRAME_BYTES 0x1200u
#define W2U_MCSS_TEX_BYTES 0x4000u
#define W2U_MCSS_TEX_WIDTH 256u
#define W2U_MCSS_TEX_STRIDE_BYTES (W2U_MCSS_TEX_WIDTH / 2u)
#define W2U_VISIBLE_TEX_WIDTH 96u
#define W2U_VISIBLE_TEX_HEIGHT 96u
#define W2U_VISIBLE_TEX_ROW_BYTES (W2U_VISIBLE_TEX_WIDTH / 2u)
#define W2U_VISIBLE_TEX_ROW_HALFWORDS (W2U_VISIBLE_TEX_ROW_BYTES / 2u)
#define W2U_VISIBLE_TEX_BYTES (W2U_VISIBLE_TEX_ROW_BYTES * W2U_VISIBLE_TEX_HEIGHT)
#define W2U_STAGING_TEX_BYTES (W2U_MCSS_TEX_STRIDE_BYTES * W2U_VISIBLE_TEX_HEIGHT)
#define W2U_LEGACY_TEX_BYTES_AVOIDED (W2U_MCSS_TEX_BYTES - W2U_VISIBLE_TEX_BYTES)
#define W2U_MCSS_TEX_BASE 0x24000u
#define W2U_MCSS_TEX_SLOT_BYTES 0x4000u
#define W2U_MCSS_TEX_SLOT_COUNT 8u
#define W2U_MCSS_PLTT_BASE 0x1000u
#define W2U_MCSS_PLTT_SLOT_BYTES 0x20u
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
#define W2U_BTLV_BEW_PTR ((void **)0x021F4280)
#define W2U_BATTLE_SPRITE_SYSTEM_OFFSET 0x190u
#define W2U_BTLV_POS_AA 0
#define W2U_BTLV_POS_BB 1
#define W2U_BTLV_POS_A 2
#define W2U_BTLV_POS_B 3
#define W2U_BTLV_POS_C 4
#define W2U_BTLV_POS_D 5
#define W2U_BTLV_POS_E 6
#define W2U_BTLV_POS_F 7
#define W2U_BATTLE_PROFILE_MAGIC 0x46525042u
#define W2U_BATTLE_PROFILE_VERSION 7u
#define W2U_BATTLE_ACTOR_ENTRY_BASE 0x08u
#define W2U_BATTLE_ACTOR_ENTRY_BYTES 0x5cu
#define W2U_BATTLE_ACTOR_SPECIES_OFFSET 0x2cu
#define W2U_MCSS_BASE_PLTT_DATA_OFFSET 0xd4u
#define W2U_MCSS_FADE_PLTT_DATA_OFFSET 0xd8u
#define W2U_MCSS_PLTT_DATA_SIZE_OFFSET 0xdcu
#define W2U_MAIN_RAM_START 0x02000000u
#define W2U_MAIN_RAM_END 0x02400000u

namespace w2u {
namespace battle_anim {

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

enum ActorId {
    ACTOR_SINGLE_PLAYER_BACK = 0,
    ACTOR_SINGLE_ENEMY_FRONT = 1,
    ACTOR_MULTI_PLAYER_0_BACK = 2,
    ACTOR_MULTI_ENEMY_0_FRONT = 3,
    ACTOR_MULTI_PLAYER_1_BACK = 4,
    ACTOR_MULTI_ENEMY_1_FRONT = 5,
    ACTOR_MULTI_PLAYER_2_BACK = 6,
    ACTOR_MULTI_ENEMY_2_FRONT = 7,
    ACTOR_COUNT = 8,
};

enum BattleAssetId {
    ASSET_NONE = 0xffffu,
};

#define ASSET_COUNT W2U_PWAN_ASSET_COUNT

struct ActorConfig {
    u8 position;
};

struct Asset {
    b32 loaded;
    b32 fileOpen;
    u32 assetId;
    FSFile file;
    PwanHeader header;
    PwanTimelineEntry timeline[128];
    u16 palette[16];
};

struct ActorState {
    b32 active;
    b32 textureDirty;
    b32 paletteDirty;
    u32 tick;
    u16 copiedFrame;
    u16 pendingFrame;
    s16 mcssIndex;
    u16 species;
    u16 assetId;
    void *mcss;
};

struct BattleAnimProfile {
    u32 magic;
    u32 version;
    u32 structSize;
    u32 drawCalls;
    u32 waitCalls;
    u32 waitSpinIterations;
    u32 maxWaitSpinIterations;
    u32 textureUploadCalls;
    u32 textureUploadActors;
    u32 textureBytesUploaded;
    u32 legacyTextureBytesAvoided;
    u32 lastUploadBytes;
    u32 lastUploadActors;
    u32 lastVcountBeforeWait;
    u32 lastVcountAfterWait;
    u32 updateCalls;
    u32 nullBewCount;
    u32 nullBmwCount;
    u32 lastBew;
    u32 lastBmw;
    u32 mcssIndexCalls;
    u32 mcssIndexValid;
    u32 mcssIndexInvalid;
    u32 lastMcssIndexPosition;
    s32 lastMcssIndexResult;
    u32 monsReadCalls;
    u32 entryNullCount;
    u32 monsInvalidCount;
    u32 lastEntry;
    u32 lastEntryMcss;
    s32 lastEntryMons;
    u32 loadFailCount;
    u32 stageFailCount;
    u32 lastLoadFailActor;
    u32 lastLoadFailAsset;
    u32 lastStageFailActor;
    u32 lastStageFailAsset;
    u32 lastStageFailFrame;
    u32 lastActiveMask;
    u32 lastTextureDirtyMask;
    u32 actorPosition[ACTOR_COUNT];
    s32 actorMcssIndex[ACTOR_COUNT];
    u32 actorSpecies[ACTOR_COUNT];
    u32 actorAsset[ACTOR_COUNT];
    u32 actorFrame[ACTOR_COUNT];
    u32 paletteUploadCalls;
    u32 paletteCpuCopyCalls;
    u32 paletteCpuCopyFailCount;
    u32 lastPaletteActor;
    u32 lastPaletteAsset;
    u32 lastPaletteMcss;
    u32 lastPaletteBase;
    u32 lastPaletteFade;
    u32 lastPaletteSize;
};

struct State {
    Asset asset[ACTOR_COUNT];
    ActorState actor[ACTOR_COUNT];
    u8 nextUploadActor;
};

extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
volatile BattleAnimProfile W2U_BattleAnim_Profile = {
    W2U_BATTLE_PROFILE_MAGIC,
    W2U_BATTLE_PROFILE_VERSION,
    sizeof(BattleAnimProfile),
};
#pragma GCC diagnostic pop
}

static const ActorConfig kActorConfig[ACTOR_COUNT] = {
    {W2U_BTLV_POS_AA},
    {W2U_BTLV_POS_BB},
    {W2U_BTLV_POS_A},
    {W2U_BTLV_POS_B},
    {W2U_BTLV_POS_C},
    {W2U_BTLV_POS_D},
    {W2U_BTLV_POS_E},
    {W2U_BTLV_POS_F},
};

static State sState;
static char sAssetPath[W2U_PWAN_PATH_BYTES];
static u8 sFrameScratch[W2U_FRAME_BYTES];
static u8 sTextureScratch[W2U_STAGING_TEX_BYTES];

typedef s32 (*McssGetIndexFn)(void *bmw, int position);

static b32 IsSafeMcssTextureIndex(s32 mcssIndex);

static McssGetIndexFn const BattleSpriteGetIndex_Fn = (McssGetIndexFn)0x021E97D5u;
extern "C" void W2U_BattleAnim_Term(void);

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

static const char *PathForAsset(BattleAssetId assetId)
{
    if ((u32)assetId >= ASSET_COUNT) return 0;
    WriteAssetPath(sAssetPath, (u32)assetId);
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

static b32 OpenAssetFile(Asset *asset, BattleAssetId assetId)
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

static b32 LoadAsset(ActorId actor, BattleAssetId assetId)
{
    if (assetId >= ASSET_COUNT) {
        return false;
    }

    Asset *asset = &sState.asset[actor];
    if (asset->loaded && asset->fileOpen && asset->assetId == (u32)assetId) {
        return true;
    }
    CloseAssetFile(asset);
    asset->loaded = false;
    asset->assetId = (u32)assetId;

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

static b32 IsLikelyMainRamPointer(const void *ptr)
{
    const u32 value = (u32)ptr;
    return value >= W2U_MAIN_RAM_START && value < W2U_MAIN_RAM_END;
}

static void *GetMcssPointerByIndex(void *bmw, s32 mcssIndex)
{
    if (!bmw || !IsSafeMcssTextureIndex(mcssIndex)) {
        return 0;
    }
    u8 *entry = (u8 *)bmw + W2U_BATTLE_ACTOR_ENTRY_BASE +
                ((u32)mcssIndex * W2U_BATTLE_ACTOR_ENTRY_BYTES);
    return *(void **)entry;
}

static void CopyPaletteToLiveMcss(ActorId actor)
{
    const BattleAssetId assetId = (BattleAssetId)sState.actor[actor].assetId;
    if (assetId >= ASSET_COUNT) {
        return;
    }

    Asset *asset = &sState.asset[actor];
    void *mcss = sState.actor[actor].mcss;
    W2U_BattleAnim_Profile.lastPaletteActor = (u32)actor;
    W2U_BattleAnim_Profile.lastPaletteAsset = (u32)assetId;
    W2U_BattleAnim_Profile.lastPaletteMcss = (u32)mcss;

    if (!IsLikelyMainRamPointer(mcss)) {
        W2U_BattleAnim_Profile.paletteCpuCopyFailCount =
            W2U_BattleAnim_Profile.paletteCpuCopyFailCount + 1u;
        return;
    }

    u16 *base = *(u16 **)((u8 *)mcss + W2U_MCSS_BASE_PLTT_DATA_OFFSET);
    u16 *fade = *(u16 **)((u8 *)mcss + W2U_MCSS_FADE_PLTT_DATA_OFFSET);
    const u32 size = *(u32 *)((u8 *)mcss + W2U_MCSS_PLTT_DATA_SIZE_OFFSET);
    W2U_BattleAnim_Profile.lastPaletteBase = (u32)base;
    W2U_BattleAnim_Profile.lastPaletteFade = (u32)fade;
    W2U_BattleAnim_Profile.lastPaletteSize = size;

    if (!IsLikelyMainRamPointer(base) || !IsLikelyMainRamPointer(fade) ||
        size < W2U_MCSS_PLTT_SLOT_BYTES || size > 0x200u) {
        W2U_BattleAnim_Profile.paletteCpuCopyFailCount =
            W2U_BattleAnim_Profile.paletteCpuCopyFailCount + 1u;
        return;
    }

    for (u32 i = 0; i < 16; ++i) {
        base[i] = asset->palette[i];
        fade[i] = asset->palette[i];
    }
    W2U_BattleAnim_Profile.paletteCpuCopyCalls =
        W2U_BattleAnim_Profile.paletteCpuCopyCalls + 1u;
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
    W2U_BattleAnim_Profile.waitCalls = W2U_BattleAnim_Profile.waitCalls + 1u;
    W2U_BattleAnim_Profile.lastVcountBeforeWait = *W2U_REG_VCOUNT;
    while (!IsSafeVramUploadTime()) {
        spins++;
    }
    W2U_BattleAnim_Profile.waitSpinIterations += spins;
    if (spins > W2U_BattleAnim_Profile.maxWaitSpinIterations) {
        W2U_BattleAnim_Profile.maxWaitSpinIterations = spins;
    }
    W2U_BattleAnim_Profile.lastVcountAfterWait = *W2U_REG_VCOUNT;
}

static void UploadPalette(ActorId actor, s32 mcssIndex)
{
    const BattleAssetId assetId = (BattleAssetId)sState.actor[actor].assetId;
    if (assetId >= ASSET_COUNT) {
        return;
    }

    Asset *asset = &sState.asset[actor];
    u8 e, f, g;
    SetTexturePaletteBanksLcdc(&e, &f, &g);

    volatile u16 *dst = W2U_LCDC_TEX_PLTT +
                        ((W2U_MCSS_PLTT_BASE + W2U_MCSS_PLTT_SLOT_BYTES * (u32)mcssIndex) / 2u);
    for (u32 i = 0; i < 16; ++i) {
        dst[i] = asset->palette[i];
    }

    RestoreTexturePaletteBanks(e, f, g);
    W2U_BattleAnim_Profile.paletteUploadCalls =
        W2U_BattleAnim_Profile.paletteUploadCalls + 1u;
    sState.actor[actor].paletteDirty = false;
}

static u16 FrameForTick(const Asset *asset, u32 tick)
{
    u32 at = 0;
    for (u32 i = 0; i < asset->header.timelineCount; ++i) {
        at += asset->timeline[i].ticks;
        if (tick < at) {
            return asset->timeline[i].frame;
        }
    }
    return asset->timeline[asset->header.timelineCount - 1].frame;
}

static void BlitTileSegmentToTexture(u8 *dst, const u8 *src, u32 dstX, u32 dstY, u32 tilesW, u32 tilesH)
{
    for (u32 tileY = 0; tileY < tilesH; ++tileY) {
        for (u32 tileX = 0; tileX < tilesW; ++tileX) {
            const u8 *tile = src + ((tileY * tilesW + tileX) * 32u);
            for (u32 y = 0; y < 8; ++y) {
                u8 *row = dst + (dstY + tileY * 8u + y) * W2U_MCSS_TEX_STRIDE_BYTES +
                          (dstX + tileX * 8u) / 2u;
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
    u8 *texture = sTextureScratch;
    const u8 *frame = sFrameScratch;
    BlitTileSegmentToTexture(texture, frame + 0x0000u, 0, 0, 8, 8);
    BlitTileSegmentToTexture(texture, frame + 0x0800u, 64, 0, 4, 8);
    BlitTileSegmentToTexture(texture, frame + 0x0c00u, 0, 64, 8, 4);
    BlitTileSegmentToTexture(texture, frame + 0x1000u, 64, 64, 4, 4);
}

static void UploadTexture(ActorId actor, s32 mcssIndex)
{
    u8 a, b, c, d;
    SetTextureBanksLcdc(&a, &b, &c, &d);

    volatile u16 *dst = W2U_LCDC_TEX_VRAM +
                        ((W2U_MCSS_TEX_BASE + W2U_MCSS_TEX_SLOT_BYTES * (u32)mcssIndex) / 2u);
    for (u32 y = 0; y < W2U_VISIBLE_TEX_HEIGHT; ++y) {
        volatile u16 *dstRow = dst + ((y * W2U_MCSS_TEX_STRIDE_BYTES) / 2u);
        const u16 *srcRow = (const u16 *)(sTextureScratch + y * W2U_MCSS_TEX_STRIDE_BYTES);
        for (u32 x = 0; x < W2U_VISIBLE_TEX_ROW_HALFWORDS; ++x) {
            dstRow[x] = srcRow[x];
        }
    }

    RestoreTextureBanks(a, b, c, d);
    W2U_BattleAnim_Profile.textureUploadActors = W2U_BattleAnim_Profile.textureUploadActors + 1u;
    W2U_BattleAnim_Profile.textureBytesUploaded += W2U_VISIBLE_TEX_BYTES;
    W2U_BattleAnim_Profile.legacyTextureBytesAvoided += W2U_LEGACY_TEX_BYTES_AVOIDED;
    W2U_BattleAnim_Profile.lastUploadActors = W2U_BattleAnim_Profile.lastUploadActors + 1u;
    W2U_BattleAnim_Profile.lastUploadBytes += W2U_VISIBLE_TEX_BYTES;
    sState.actor[actor].textureDirty = false;
}

static b32 StageFrameTexture(ActorId actor, u16 frame)
{
    const BattleAssetId assetId = (BattleAssetId)sState.actor[actor].assetId;
    if (assetId >= ASSET_COUNT) {
        W2U_BattleAnim_Profile.stageFailCount = W2U_BattleAnim_Profile.stageFailCount + 1u;
        W2U_BattleAnim_Profile.lastStageFailActor = (u32)actor;
        W2U_BattleAnim_Profile.lastStageFailAsset = (u32)assetId;
        W2U_BattleAnim_Profile.lastStageFailFrame = frame;
        return false;
    }

    Asset *asset = &sState.asset[actor];
    const u32 offset = asset->header.frameOffset + (frame * asset->header.frameBytes);
    if (!ReadAssetRange(asset, offset, sFrameScratch, W2U_FRAME_BYTES)) {
        W2U_BattleAnim_Profile.stageFailCount = W2U_BattleAnim_Profile.stageFailCount + 1u;
        W2U_BattleAnim_Profile.lastStageFailActor = (u32)actor;
        W2U_BattleAnim_Profile.lastStageFailAsset = (u32)assetId;
        W2U_BattleAnim_Profile.lastStageFailFrame = frame;
        return false;
    }

    ConvertFrameToTexture();
    return true;
}

static void *GetMcssWork()
{
    void *bew = *W2U_BTLV_BEW_PTR;
    W2U_BattleAnim_Profile.lastBew = (u32)bew;
    if (!bew) {
        W2U_BattleAnim_Profile.nullBewCount = W2U_BattleAnim_Profile.nullBewCount + 1u;
        W2U_BattleAnim_Profile.lastBmw = 0;
        return 0;
    }
    void *bmw = *(void **)((u8 *)bew + W2U_BATTLE_SPRITE_SYSTEM_OFFSET);
    W2U_BattleAnim_Profile.lastBmw = (u32)bmw;
    if (!bmw) {
        W2U_BattleAnim_Profile.nullBmwCount = W2U_BattleAnim_Profile.nullBmwCount + 1u;
    }
    return bmw;
}

static s32 GetMcssIndex(void *bmw, int position)
{
    W2U_BattleAnim_Profile.mcssIndexCalls = W2U_BattleAnim_Profile.mcssIndexCalls + 1u;
    W2U_BattleAnim_Profile.lastMcssIndexPosition = (u32)position;
    if (!bmw) {
        W2U_BattleAnim_Profile.mcssIndexInvalid = W2U_BattleAnim_Profile.mcssIndexInvalid + 1u;
        W2U_BattleAnim_Profile.lastMcssIndexResult = -1;
        return -1;
    }
    const s32 index = BattleSpriteGetIndex_Fn(bmw, position);
    W2U_BattleAnim_Profile.lastMcssIndexResult = index;
    if (index >= 0 && (u32)index < W2U_MCSS_TEX_SLOT_COUNT) {
        W2U_BattleAnim_Profile.mcssIndexValid = W2U_BattleAnim_Profile.mcssIndexValid + 1u;
    } else {
        W2U_BattleAnim_Profile.mcssIndexInvalid = W2U_BattleAnim_Profile.mcssIndexInvalid + 1u;
    }
    return index;
}

static s32 GetMcssMonsNo(void *bmw, int position)
{
    W2U_BattleAnim_Profile.monsReadCalls = W2U_BattleAnim_Profile.monsReadCalls + 1u;
    if (!bmw) {
        W2U_BattleAnim_Profile.monsInvalidCount = W2U_BattleAnim_Profile.monsInvalidCount + 1u;
        return SPECIES_NONE;
    }

    const s32 index = GetMcssIndex(bmw, position);
    if (index < 0) {
        W2U_BattleAnim_Profile.monsInvalidCount = W2U_BattleAnim_Profile.monsInvalidCount + 1u;
        return SPECIES_NONE;
    }

    u8 *entry = (u8 *)bmw + W2U_BATTLE_ACTOR_ENTRY_BASE +
                ((u32)index * W2U_BATTLE_ACTOR_ENTRY_BYTES);
    W2U_BattleAnim_Profile.lastEntry = (u32)entry;
    W2U_BattleAnim_Profile.lastEntryMcss = (u32)(*(void **)entry);
    if (*(void **)entry == 0) {
        W2U_BattleAnim_Profile.entryNullCount = W2U_BattleAnim_Profile.entryNullCount + 1u;
        W2U_BattleAnim_Profile.monsInvalidCount = W2U_BattleAnim_Profile.monsInvalidCount + 1u;
        return SPECIES_NONE;
    }

    const s32 monsNo = *(s32 *)(entry + W2U_BATTLE_ACTOR_SPECIES_OFFSET);
    W2U_BattleAnim_Profile.lastEntryMons = monsNo;
    if (monsNo <= SPECIES_NONE) {
        W2U_BattleAnim_Profile.monsInvalidCount = W2U_BattleAnim_Profile.monsInvalidCount + 1u;
    }
    return monsNo;
}

static BattleAssetId GetAssetForSpeciesSide(u16 species, b32 isFront)
{
    const u32 assetId = w2u::pwan::GetAssetForSpeciesSide(species, isFront);
    return assetId == W2U_PWAN_CONFIG_ASSET_NONE ? ASSET_NONE : (BattleAssetId)assetId;
}

static BattleAssetId GetAssetForPositionSpecies(int position, u16 species)
{
    if ((position & 1) == 0) {
        return GetAssetForSpeciesSide(species, false);
    }

    return GetAssetForSpeciesSide(species, true);
}

static b32 IsSafeMcssTextureIndex(s32 mcssIndex)
{
    return mcssIndex >= 0 && (u32)mcssIndex < W2U_MCSS_TEX_SLOT_COUNT;
}

static void RecordActorProfile(ActorId actor, u32 position, s32 mcssIndex, u16 species,
                               BattleAssetId assetId, b32 active, b32 textureDirty, u16 frame)
{
    W2U_BattleAnim_Profile.actorPosition[actor] = position;
    W2U_BattleAnim_Profile.actorMcssIndex[actor] = mcssIndex;
    W2U_BattleAnim_Profile.actorSpecies[actor] = species;
    W2U_BattleAnim_Profile.actorAsset[actor] = (u32)assetId;
    W2U_BattleAnim_Profile.actorFrame[actor] = frame;
    if (active) {
        W2U_BattleAnim_Profile.lastActiveMask |= (1u << (u32)actor);
    }
    if (textureDirty) {
        W2U_BattleAnim_Profile.lastTextureDirtyMask |= (1u << (u32)actor);
    }
}

static void DeactivateActor(ActorId actor)
{
    Asset *asset = &sState.asset[actor];
    CloseAssetFile(asset);
    asset->loaded = false;
    asset->assetId = ASSET_NONE;
    sState.actor[actor].active = false;
    sState.actor[actor].textureDirty = false;
    sState.actor[actor].paletteDirty = false;
    sState.actor[actor].copiedFrame = 0xffffu;
    sState.actor[actor].pendingFrame = 0xffffu;
    sState.actor[actor].mcssIndex = -1;
    sState.actor[actor].species = SPECIES_NONE;
    sState.actor[actor].assetId = ASSET_NONE;
    sState.actor[actor].mcss = 0;
}

static void UpdateActor(ActorId actor, void *bmw)
{
    const ActorConfig *cfg = &kActorConfig[actor];
    ActorState *actorState = &sState.actor[actor];

    const s32 mcssIndex = GetMcssIndex(bmw, cfg->position);
    if (!IsSafeMcssTextureIndex(mcssIndex)) {
        RecordActorProfile(actor, cfg->position, mcssIndex, SPECIES_NONE, ASSET_NONE, false, false, 0xffffu);
        DeactivateActor(actor);
        return;
    }
    const s32 speciesNo = GetMcssMonsNo(bmw, cfg->position);
    const u16 species = (speciesNo > SPECIES_NONE && speciesNo <= 0xffff) ?
                        (u16)speciesNo : (u16)SPECIES_NONE;
    const BattleAssetId assetId = GetAssetForPositionSpecies(cfg->position, species);
    if (assetId >= ASSET_COUNT || !LoadAsset(actor, assetId)) {
        W2U_BattleAnim_Profile.loadFailCount = W2U_BattleAnim_Profile.loadFailCount + 1u;
        W2U_BattleAnim_Profile.lastLoadFailActor = (u32)actor;
        W2U_BattleAnim_Profile.lastLoadFailAsset = (u32)assetId;
        RecordActorProfile(actor, cfg->position, mcssIndex, species, assetId, false, false, 0xffffu);
        DeactivateActor(actor);
        return;
    }

    const b32 wasInactive = !actorState->active;
    const b32 mcssIndexChanged = actorState->mcssIndex != (s16)mcssIndex;
    const b32 speciesChanged = actorState->species != species || actorState->assetId != assetId;
    if (speciesChanged) {
        actorState->tick = 0;
        actorState->copiedFrame = 0xffffu;
        actorState->pendingFrame = 0xffffu;
    }

    actorState->active = true;
    actorState->mcssIndex = (s16)mcssIndex;
    actorState->species = species;
    actorState->assetId = (u16)assetId;
    actorState->mcss = GetMcssPointerByIndex(bmw, mcssIndex);
    if (wasInactive || mcssIndexChanged || speciesChanged) {
        actorState->paletteDirty = true;
    }
    if (actorState->paletteDirty) {
        CopyPaletteToLiveMcss(actor);
    }

    Asset *asset = &sState.asset[actor];
    const u32 totalTicks = asset->header.totalTicks ? asset->header.totalTicks : 1;
    if (actorState->tick >= totalTicks) {
        actorState->tick = 0;
    }
    const u16 frame = FrameForTick(asset, actorState->tick);
    if (wasInactive || mcssIndexChanged || speciesChanged || frame != actorState->copiedFrame) {
        actorState->pendingFrame = frame;
        actorState->textureDirty = true;
    }
    RecordActorProfile(actor, cfg->position, mcssIndex, species, assetId, true,
                       actorState->textureDirty, frame);
    actorState->tick = actorState->tick + 1u;
    if (actorState->tick >= totalTicks) {
        actorState->tick = 0;
    }
}

extern "C" void W2U_BattleAnim_Update(void)
{
    W2U_BattleAnim_Profile.updateCalls = W2U_BattleAnim_Profile.updateCalls + 1u;
    W2U_BattleAnim_Profile.lastActiveMask = 0;
    W2U_BattleAnim_Profile.lastTextureDirtyMask = 0;
    void *bmw = GetMcssWork();
    if (!bmw) {
        W2U_BattleAnim_Term();
        return;
    }

    for (u32 i = 0; i < ACTOR_COUNT; ++i) {
        UpdateActor((ActorId)i, bmw);
    }
}

extern "C" void W2U_BattleAnim_Draw(void)
{
    W2U_BattleAnim_Profile.drawCalls = W2U_BattleAnim_Profile.drawCalls + 1u;

    b32 needsTextureUpload = false;
    for (u32 i = 0; i < ACTOR_COUNT; ++i) {
        if (!sState.actor[i].active) {
            continue;
        }
        if (sState.actor[i].textureDirty) {
            needsTextureUpload = true;
        }
    }
    if (!needsTextureUpload) {
        return;
    }

    W2U_BattleAnim_Profile.lastUploadBytes = 0;
    W2U_BattleAnim_Profile.lastUploadActors = 0;
    W2U_BattleAnim_Profile.textureUploadCalls = W2U_BattleAnim_Profile.textureUploadCalls + 1u;

    for (u32 attempt = 0; attempt < ACTOR_COUNT; ++attempt) {
        u32 actorIndex = sState.nextUploadActor + attempt;
        while (actorIndex >= ACTOR_COUNT) {
            actorIndex -= ACTOR_COUNT;
        }
        const ActorId actor = (ActorId)actorIndex;
        ActorState *actorState = &sState.actor[actor];
        if (!actorState->active || actorState->mcssIndex < 0 || !actorState->textureDirty) {
            continue;
        }

        if (StageFrameTexture(actor, actorState->pendingFrame)) {
            WaitForSafeVramUploadTime();
            if (actorState->paletteDirty) {
                CopyPaletteToLiveMcss(actor);
                UploadPalette(actor, actorState->mcssIndex);
            }
            UploadTexture(actor, actorState->mcssIndex);
            actorState->copiedFrame = actorState->pendingFrame;
        } else {
            actorState->textureDirty = false;
            actorState->paletteDirty = false;
        }
        actorIndex = actorIndex + 1u;
        if (actorIndex >= ACTOR_COUNT) {
            actorIndex = 0;
        }
        sState.nextUploadActor = (u8)actorIndex;
        break;
    }
}

extern "C" void W2U_BattleAnim_Term(void)
{
    for (u32 i = 0; i < ACTOR_COUNT; ++i) {
        CloseAssetFile(&sState.asset[i]);
        sState.asset[i].loaded = false;
        sState.asset[i].assetId = ASSET_NONE;
        sState.actor[i].active = false;
        sState.actor[i].textureDirty = false;
        sState.actor[i].paletteDirty = false;
        sState.actor[i].tick = 0;
        sState.actor[i].copiedFrame = 0xffffu;
        sState.actor[i].pendingFrame = 0xffffu;
        sState.actor[i].mcssIndex = -1;
        sState.actor[i].mcss = 0;
    }
    sState.nextUploadActor = 0;
}

} // namespace battle_anim
} // namespace w2u
