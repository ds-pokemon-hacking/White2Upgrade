#include "swan/swantypes.h"
#include "gfl/fs/gfl_archive.h"
#include "Species.h"
#include "Personal.h"
#include "FileSystem.h"

#define REGIONAL_DEX_FILE_INDEX 826

#define POKE_FORM_LIST_SIZE 0x48

namespace w2u {
    namespace pml {

        struct Poke_form
        {
            u16 species;
            u16 form_count;
        };

        struct PokedexSave {
			u8 gap_0[428];
			u8 seenMale[84];
			u8 seenFemale[84];
			u8 shinyMale[84];
			u8 shinyFemale[84];
			u8 gap_2FC[22];
			u8 forms[11];
			u8 shinyForms[11];
		};

        extern "C" b32 PokeDex_IsCaught(void *pDexAddress, u16 species);
        extern "C" b32 PokeDex_IsSeen(void *pDexAddress, u16 species);
        extern "C" b32 PML_PkmIsRegionalDexExclude(u32 species);
        extern "C" b32 PML_PkmIsNationalDexExclude(u32 species);
        extern "C" u16 *PML_PersonalLoadRegionalDexTable(HeapID heapId, u16 *regionalDexCount);
        extern "C" s16 getIndexNumOfPkmForm(u16 species);
		extern "C" s32 getIndexPokemonWithForms(u32 species);
		extern "C" PersonalData* PML_PersonalLoad(u16 species, u16 form, u16 heapId);
		extern "C" u32 PML_PersonalGetParam(PersonalData* personal, PersonalField field);
		extern "C" void PML_PersonalFree(PersonalData* personal);

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

        extern "C" s32 THUMB_BRANCH_getIndexPokemonWithForms(u32 species) {
			Poke_form pokemonFormList[POKE_FORM_LIST_SIZE];
			if (!ReadDataFromFile("poke_form_list.bin", POKE_FORM_LIST_SIZE * sizeof(Poke_form), (u8*)pokemonFormList)) {
				return -1;
			}
			
            s32 formIdx = 0;
			for (u32 i = 0; i < POKE_FORM_LIST_SIZE; ++i)
            {
                Poke_form *form = &pokemonFormList[i];
                if ( species == form->species ) {
					return formIdx;
				}

                formIdx += form->form_count;
            }
			return -1;
		}  

        extern "C" s16 THUMB_BRANCH_getIndexNumOfPkmForm(u16 species) {
			Poke_form pokemonFormList[POKE_FORM_LIST_SIZE];
			if (!ReadDataFromFile("poke_form_list.bin", POKE_FORM_LIST_SIZE * sizeof(Poke_form), (u8*)pokemonFormList)) {
				return -1;
			}

            for (u32 i = 0; i < POKE_FORM_LIST_SIZE; ++i) {
                if ( species == pokemonFormList[i].species) {
                    return i;
                }
            }

			return -1;
		}

        extern "C" s16 THUMB_BRANCH_getNumberOfForms(u16 species)
		{
			Poke_form pokemonFormList[POKE_FORM_LIST_SIZE];
			if (!ReadDataFromFile("poke_form_list.bin", POKE_FORM_LIST_SIZE * sizeof(Poke_form), (u8*)pokemonFormList)) {
				return -1;
			}

			s16 formListIdx = getIndexNumOfPkmForm(species);
			if ( formListIdx == -1 ) {
				return 1;
			}
			else {
				return pokemonFormList[formListIdx].form_count;
			}
		}
		
		extern "C" u32 THUMB_BRANCH_SAFESTACK_addToDex(PokedexSave* pokedex, u32 species, u32 sex, u32 shiny, u32 form) {
			Poke_form pokemonFormList[POKE_FORM_LIST_SIZE];
			if (!ReadDataFromFile("poke_form_list.bin", POKE_FORM_LIST_SIZE * sizeof(Poke_form), (u8*)pokemonFormList)) {
				return -1;
			}
		
			s32 formIdx = 0;
			u32 formListIdx = 0;
			// Find the index of the first form of the Pokémon.
			while (1) {
				Poke_form* pokeForm = &pokemonFormList[formListIdx];
				if (species == pokeForm->species) {
					break;
				}
				formIdx += pokeForm->form_count;
		
				++formListIdx;
				if (!pokemonFormList[formListIdx].species) {
					formIdx = -1;
					break;
				}
			}
		
			// If the Pokémon has a form.
			if (formIdx != -1) {
				u32 formCount = pokemonFormList[formListIdx].form_count;
		
				// Reset all form flags to 0.
				for (u32 currentForm = 0; currentForm < formCount; ++currentForm) {
					u32 formGroup = (formIdx + currentForm) >> 3;
		
					u8 formMask = (u8)(1 << ((formIdx + currentForm) & 7));
					formMask = ~formMask;
		
					pokedex->forms[formGroup] &= formMask;
					pokedex->shinyForms[formGroup] &= formMask;
				}
				if (form >= formCount)
				{
					form = 0;
				}
		
				formIdx = formIdx + form;
				// Set as seen the current form flag.
				if (shiny == 1) {
					pokedex->shinyForms[formIdx >> 3] |= 1 << (formIdx & 7);
				}
				else {
					pokedex->forms[formIdx >> 3] |= 1 << (formIdx & 7);
				}
			}
		
			u32 group = (species - 1) >> 3;
			u8 mask = (u8)(1 << ((species - 1) & 7));
		
			// Reset all this Pokémons seen flags to 0.
			pokedex->seenMale[group] &= ~mask;
			pokedex->seenFemale[group] &= ~mask;
			pokedex->shinyMale[group] &= ~mask;
			pokedex->shinyFemale[group] &= ~mask;
		
			// Set the correct seen flag.
			if (sex) {
				if (sex == 1) {
					if (shiny == 1) {
						pokedex->shinyFemale[group] |= mask;// female shiny
					}
					else {
						pokedex->seenFemale[group] |= mask;// female
					}
				}
				else if (shiny == 1) {
					pokedex->shinyMale[group] |= mask;// male shiny
				}
				else {
					pokedex->seenMale[group] |= mask;// male
				}
			}
			else if (shiny == 1) {
				pokedex->shinyMale[group] |= mask;// no-sex shiny
			}
			else {
				pokedex->seenMale[group] |= mask;  // no-sex
			}
		
			return group;
		}
		
		extern "C" void THUMB_BRANCH_SAFESTACK_GetPkmDataFromPokedex(PokedexSave* pokedex, int species, int* sex, int* shiny, int* form, u16 heapID) {
			// Find the index of the first form of the Pokémon.
			s16 formListIdx = getIndexNumOfPkmForm(species);
			u32 group = (species - 1) >> 3;
			u8 mask = (u8)(1 << ((species - 1) & 7));

			// This presets all the values of the Pokémon to a usable value in case no flags are found.
			// This happens because the save file has not been extended and the PokéDex flags get all messed up.
			*sex = 0;
			*shiny = 0;
			*form = 0;
		
			// If the Pokémon does NOT have a form.
			if (formListIdx == -1) {
				*form = 0;
				if ((pokedex->seenMale[group] & mask) != 0) {
					*sex = 0;
					*shiny = 0;
				}
				if ((pokedex->seenFemale[group] & mask) != 0) {
					*sex = 1;
					*shiny = 0;
				}
				if ((pokedex->shinyMale[group] & mask) != 0) {
					*sex = 0;
					*shiny = 1;
				}
				if ((pokedex->shinyFemale[group] & mask) != 0) {
					*sex = 1;
					*shiny = 1;
				}
			}
			else {
				s32 formIdx = getIndexPokemonWithForms(species);
		
				Poke_form pokemonFormList[POKE_FORM_LIST_SIZE];
				if (!ReadDataFromFile("poke_form_list.bin", POKE_FORM_LIST_SIZE * sizeof(Poke_form), (u8*)pokemonFormList)) {
					return;
				}
		
				u16 formCount = pokemonFormList[formListIdx].form_count;
				if (formCount) {
					for (u32 currentForm = 0; currentForm < formCount; ++currentForm) {
						u32 formGroup = (formIdx + currentForm) >> 3;
						u8 formMask = (u8)(1 << ((formIdx + currentForm) & 7));
		
						if ((pokedex->forms[formGroup] & formMask) != 0) {
							*form = currentForm;
							*shiny = 0;
							break;
						}
						if ((pokedex->shinyForms[formGroup] & formMask) != 0) {
							*form = currentForm;
							*shiny = 1;
							break;
						}
					}
				}
		
				if ((pokedex->seenMale[group] & mask) != 0
					|| (pokedex->shinyMale[group] & mask) != 0) {
					*sex = 0;
				}
				if ((pokedex->seenFemale[group] & mask) != 0
					|| (pokedex->shinyFemale[group] & mask) != 0) {
					*sex = 1;
				}
			}
		
			PersonalData* personal = PML_PersonalLoad(species, *form, heapID);
			if (PML_PersonalGetParam(personal, Personal_GenderProb) == 255)
			{
				*sex = 2;
			}
			PML_PersonalFree(personal);
		}
    }
}