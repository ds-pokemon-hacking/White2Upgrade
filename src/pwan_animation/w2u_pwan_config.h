#ifndef W2U_PWAN_CONFIG_H
#define W2U_PWAN_CONFIG_H

#include "swantypes.h"

#define W2U_PWAN_CONFIG_ASSET_NONE 0xffffffffu

namespace w2u {
namespace pwan {

u32 GetAssetForSpeciesSide(u16 species, b32 isFront);

} // namespace pwan
} // namespace w2u

#endif
