#include "personal.h"
#include "species.h"
#include "util/filesystem.h"

#include "pml/poke_param.h"
#include "pml/poke_party.h"
#include "pml/poke_data.h"
#include "gfl/fs/gfl_archive.h"

#define EGG_INDEX 722

#define FORM_START 14480
#define RARE_FORM_START 17953
#define REGIONAL_DEX_FILE_INDEX 826

#define ICON_FORM_START 1456

namespace w2u {
    namespace pokegra {
        using PersonalGetParamSingleFn = u32 (*)(u16 species, u16 form, PersonalField field);
        using ArcSysGetDataLengthFn = u32 (*)(int arcId, u16 datId);

        static inline u32 PML_PersonalGetParamSingleAbs(u16 species, u16 form, PersonalField field) {
            return reinterpret_cast<PersonalGetParamSingleFn>(0x0201EF49)(species, form, field);
        }

        static inline u32 GFL_ArcSysGetDataLengthAbs(int arcId, u16 datId) {
            return reinterpret_cast<ArcSysGetDataLengthFn>(0x0204AA31)(arcId, datId);
        }

        extern "C" void THUMB_BRANCH_SAFESTACK_GetPokemonDataIDBase(u32 ARCID, u32 Species, u32 Form, u32 Gender, b32 isRare, b32 isBackSprite, b32 isEgg, u32 *SpeciesData, u32 *OffsetBase, u32 *pGender, u32 *pValidRarity, u32 *pValidRareForme, b32 linearGraphics) {
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
                u32 FormCount = PML_PersonalGetParamSingleAbs(Species, 0, Personal_FormeCount);
                u32 FormSpriteOffset = PML_PersonalGetParamSingleAbs(Species, 0, Personal_FormeSpritesOffset);
                u32 SpriteForme = PML_PersonalGetParamSingleAbs(Species, 0, Personal_SpriteForme);

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
            switch (Gender) {
            case 1:
                // In case of female Pokemon, check for alternate gender sprite.
                if (!GFL_ArcSysGetDataLengthAbs(ARCID, actual_index + 1)) {
                    // Set to default if ther is none.
                    Gender = 0;
                }
                break;
            case 2:
                // Set genderless Pokemon to default.
                Gender = 0;
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
                *pGender = Gender;
            }

            if (pValidRarity) {
                *pValidRarity = isRare;
            }
        }

        extern "C" s32 THUMB_BRANCH_PokeParty_GetIconIndex(u32 Species, u32 Form, u32 Gender, u32 isEgg) {
			// An actual Pokémon; calculate its icon index.
            // Should be, by default, 2 * Species + 8.
            u32 iconIndex = 2 * Species + 8;
            
            if (isEgg) {
                // Egg; check if it is Manaphy first.
                u32 species_is_manaphy = Species == SPECIES_MANAPHY;
                // Calculate the new index.
                iconIndex = 2 * (species_is_manaphy + EGG_INDEX) + 8;
            }
            else if (Form) {
                // Handle forms.
                // The starting index has been pushed back.
                u32 formCount = PML_PersonalGetParamSingleAbs(Species, 0, Personal_FormeCount);
                u32 formSpriteOffset = PML_PersonalGetParamSingleAbs(Species, 0, Personal_FormeSpritesOffset);
                u32 formSprite = PML_PersonalGetParamSingleAbs(Species, 0, Personal_SpriteForme);
                // Forme is valid.
				if (Form < formCount && !formSprite) {
                    iconIndex = 2 * (formSpriteOffset + Form - 1) + ICON_FORM_START;
                }
            }
			
            // Handle the gender attributes.
            switch (Gender) {
            case 1:
                // In case of female Pokemon, check for alternate gender icon.
                if (!GFL_ArcSysGetDataLengthAbs(7u, iconIndex + 1)) {
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

		extern "C" u32 THUMB_BRANCH_PokeParty_GetIconPalette(u32 Species, u32 Form, u32 Gender, u32 IsEgg) {
			// The palette index and species match unless there are any special cases
			u32 paletteIndex = Species;

			if (IsEgg) {
				// Egg; check if it is Manaphy first.
				u32 isManaphy = Species == 490;
				// Calculate the new index.
				paletteIndex = isManaphy + EGG_INDEX;
			}
			else if (Form) {
				// Handle forms.
                // The starting index has been pushed back.
				u32 formSpriteOffset = PML_PersonalGetParamSingleAbs(Species, 0, Personal_FormeSpritesOffset);
				u32 formSprite = PML_PersonalGetParamSingleAbs(Species, 0, Personal_SpriteForme);
				u32 formCount = PML_PersonalGetParamSingleAbs(Species, 0, Personal_FormeCount);
				// Form is valid.
				if (Form < formCount && !formSprite) {
					paletteIndex = formSpriteOffset + (Form - 1) + (EGG_INDEX + 2);
				}   
				
			}

			u32 palette = ReadByteFromFile("pokeicon_palette_map.bin", paletteIndex);
			// Each Pokémon entry has 2 posible palettes, first 4 bits for male and the last 4 bits for female.
			// (this is only used for frillish and jellycent in vanilla but the new icons don't make use of if for now)
			if (Gender) {
				palette = (palette & 0xF0u) >> 4;
			}
			else {
				palette = palette & 0xF;
			}
			return palette;
		}
    }
}
