#include "nds/fs.h"
#include "w2u_pwan_config.h"

#define W2U_PWAN_CONFIG_PATH "pokeweb_pwan/config.bin"
#define W2U_PWAN_CONFIG_MAGIC 0x434E5750u
#define W2U_PWAN_CONFIG_VERSION 1u
#define W2U_PWAN_MAX_OVERRIDES 500u

namespace w2u {
namespace pwan {

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

struct ConfigCache {
    b32 loaded;
    u16 count;
    PwanConfigEntry entries[W2U_PWAN_MAX_OVERRIDES];
};

static ConfigCache sConfig;

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

static b32 LoadConfig(void)
{
    if (sConfig.loaded) {
        return true;
    }

    PwanConfigHeader header;
    if (!ReadPathRange(W2U_PWAN_CONFIG_PATH, 0, &header, sizeof(header)) ||
        header.magic != W2U_PWAN_CONFIG_MAGIC ||
        header.version != W2U_PWAN_CONFIG_VERSION ||
        header.count > W2U_PWAN_MAX_OVERRIDES ||
        header.maxTimeline > 128u ||
        header.entriesOffset < sizeof(PwanConfigHeader)) {
        sConfig.count = 0;
        return false;
    }

    if (header.count != 0 &&
        !ReadPathRange(W2U_PWAN_CONFIG_PATH, header.entriesOffset, sConfig.entries,
                       header.count * sizeof(PwanConfigEntry))) {
        sConfig.count = 0;
        return false;
    }

    sConfig.count = header.count;
    sConfig.loaded = true;
    return true;
}

u32 GetAssetForSpeciesSide(u16 species, b32 isFront)
{
    if (!LoadConfig()) {
        return W2U_PWAN_CONFIG_ASSET_NONE;
    }
    for (u32 i = 0; i < sConfig.count; ++i) {
        const PwanConfigEntry entry = sConfig.entries[i];
        if (entry.species != species) continue;
        if (isFront) {
            if ((entry.flags & 1u) == 0 || entry.frontIndex >= sConfig.count) {
                return W2U_PWAN_CONFIG_ASSET_NONE;
            }
            return entry.frontIndex * 2u;
        }
        if ((entry.flags & 2u) == 0 || entry.backIndex >= sConfig.count) {
            return W2U_PWAN_CONFIG_ASSET_NONE;
        }
        return entry.backIndex * 2u + 1u;
    }
    return W2U_PWAN_CONFIG_ASSET_NONE;
}

} // namespace pwan
} // namespace w2u
