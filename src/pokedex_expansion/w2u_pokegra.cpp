#include "Personal.h"
#include "Species.h"
#include "pml/poke_param.h"
#include "pml/poke_party.h"
#include "pml/poke_data.h"
#include "gfl/fs/gfl_archive.h"

#define EGG_INDEX 722

#define FORM_START 14480
#define RARE_FORM_START 17953
#define REGIONAL_DEX_FILE_INDEX 826

#define FORM_ICON_START 1456

namespace w2u {
    namespace pokegra {
        extern "C" u32 PML_PersonalGetParamSingle(u32, u32, u32);
        extern "C" void THUMB_BRANCH_SAFESTACK_GetPokemonDataIDBase(int ARCID, int Species, int Form, int Sex, b32 isRare, b32 isBackSprite, int isEgg, u32 *SpeciesData, u32 *OffsetBase, u32 *pGender, u32 *pValidRarity, u32 *pValidRareForme, b32 linearGraphics) {
            u32 actual_index = 0;
            // An actual Pokémon; calculate its base index.
            // Should be, by default, 20 * Species.
            u32 expected_index = 20 * Species;

            // There are 9 files for the front, and 9 for the back.
            // Two palettes are shared.
            u32 backsprite_shift = isBackSprite ? 9 : 0;

            if (isEgg) {
                // Egg; check if it is Manaphy first.
                u32 species_is_manaphy = Species == SPECIES_MANAPHY;
                // Calculate the new index.
                expected_index = 20 * (species_is_manaphy + EGG_INDEX);
                actual_index = expected_index;
            }
            else {
                // Index 0 of the sprite set is the linear tiled image.
                // Index 2 of the sprite set is the horizontally tiled image.
                u32 linear_shift = linearGraphics ? 0 : 2;

                // Calculate the new index.
                actual_index = expected_index + backsprite_shift + linear_shift;
            }

            // Handle forms.
            // In our case, we pushed the form data back.
            if (Form) {
                u32 FormCount = PML_PersonalGetParamSingle(Species, 0, Personal_FormeCount);
                u32 FormSpriteOffset = PML_PersonalGetParamSingle(Species, 0, Personal_FormeSpritesOffset);
                u32 SpriteForme = PML_PersonalGetParamSingle(Species, 0, Personal_SpriteForme);

                if (Form < FormCount) {
                    // Form is valid. Check if it is a rare forme.
                    if (SpriteForme) {
                        if (pValidRareForme) {
                            *pValidRareForme = isRare + 2 * (FormSpriteOffset + Form - 1) + RARE_FORM_START;
                        }
                    }
                    else {
                        expected_index = 20 * (FormSpriteOffset + Form - 1) + FORM_START;
                        actual_index = expected_index;
                    }
                }
            }

            // Handle the gender attributes.
            switch (Sex) {
            case 1:
                // In case of female Pokemon, check for alternate gender sprite.
                if (!GFL_ArcSysGetDataLength(ARCID, actual_index + 1)) {
                    // Set to default if ther is none.
                    Sex = 0;
                }
                break;
            case 2:
                // Set genderless Pokemon to default.
                Sex = 0;
                break;
            }

            // Set the pointers for the resultant data.
            if (SpeciesData) {
                *SpeciesData = expected_index;
            }

            if (OffsetBase) {
                *OffsetBase = backsprite_shift;
            }

            if (pGender) {
                *pGender = Sex;
            }

            if (pValidRarity) {
                *pValidRarity = isRare;
            }       
        }

        extern "C" s32 THUMB_BRANCH_PokeParty_GetIconIndex(s32 Species, s32 Form, s32 Gender, s32 isEgg) {
            // An actual Pokémon; calculate its icon index.
            // Should be, by default, 2 * Species + 8.
            s32 iconIndex = 2 * Species + 8;
            
            if (isEgg) {
                // Egg; check if it is Manaphy first.
                u32 species_is_manaphy = Species == SPECIES_MANAPHY;
                // Calculate the new index.
                iconIndex = 2 * (species_is_manaphy + EGG_INDEX) + 8;
            }
            else if (Form) {
                // Handle forms.
                // The starting index has been pushed back.
                u32 formeCount = PML_PersonalGetParamSingle(Species, 0, Personal_FormeCount);
                u32 formeSpriteOffset = PML_PersonalGetParamSingle(Species, 0, Personal_FormeSpritesOffset);
                u32 formeSprite = PML_PersonalGetParamSingle(Species, 0, Personal_SpriteForme);
                // Forme is valid.
                if (Form < formeCount && !formeSprite) {
                    iconIndex = 2 * (formeSpriteOffset + Form - 1) + FORM_ICON_START;
                }
            }

            // Handle the gender attributes.
            switch (Gender) {
            case 1:
                // In case of female Pokemon, check for alternate gender icon.
                if (!GFL_ArcSysGetDataLength(7u, iconIndex + 1)) {
                    // Set to default if ther is none.
                    Gender = 0;
                }
                break;
            case 2:
                // Set genderless Pokemon to default.
                Gender = 0;
                break;
            }

            return iconIndex + Gender;
        }
    }
}