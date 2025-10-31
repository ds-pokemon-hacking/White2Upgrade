from struct import unpack
from pathlib import Path

PersonalExt = Path('learnsets')

MoveNames = []
SpeciesNames = []

with open('txtdmp/Moves.txt') as Moves:
    while (CurrMove := Moves.readline()) != '':
        MoveNames.append(CurrMove.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('include/Learnsets.h', 'w') as Personal:
  Personal.write(f'#ifndef __LEARNSET_H\n#define __LEARNSET_H\n\n')
  Personal.write(f'#include "swan/swantypes.h"\n')
  Personal.write('''
#define LEVEL_UP_MOVE(MOVE, LEVEL) ((MOVE << 0x10) | LEVEL)
#define LEVEL_UP_END 0xFFFFFFFF
struct LearnsetEntry {
    uint16_t Move;
    uint16_t Level;
};
typedef struct {
    LearnsetEntry Entries[0x20];
} LEARNSET_DATA;
''')
  Personal.write(f'\n#endif\n')

with open('src/arc/pml/Learnsets.c', 'w') as Personal:
  Personal.write(f'#include "Moves.h"\n#include "Learnsets.h"\n#include "Species.h"\n\nu32 __size = sizeof(LEARNSET_DATA);\n\nconst LEARNSET_DATA __data[] = {{\n')
  Index = 0
  for Entry in sorted(PersonalExt.glob('*')):
    Personal.write(f'\t[SPECIES_{SpeciesNames[Index] if Index < len(SpeciesNames) else str(Index)}] = {{\n') # Header
    Personal.write(f'\t\t.Entries = {{\n')
    with Entry.open('rb') as PersonalRAW:
        while True:
            Mv, Lvl = unpack("<HH", PersonalRAW.read(4))
            if Lvl == 0xFFFF and Mv == 0xFFFF:
              Personal.write(f'\t\t\tLEVEL_UP_END\n')
              break
            Personal.write(f'\t\t\tLEVEL_UP_MOVE(MOVE_{MoveNames[Mv]}, {Lvl}),\n')

        PersonalRAW.close()
    Personal.write(f'\t\t}},\n')
    Personal.write(f'\t}},\n')
    PersonalRAW.close()
    Index += 1
  Personal.write(f'}};')
  Personal.close()
