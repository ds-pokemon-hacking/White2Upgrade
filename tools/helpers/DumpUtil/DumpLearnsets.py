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

Index = 0
for Entry in sorted(PersonalExt.glob('*')):
    with open(f'learnset_txt/{Entry.stem[-3:]}.yml', 'w') as PERSONAL_ENTRY:      
      PERSONAL_ENTRY.write(f'SPECIES_{SpeciesNames[Index] if Index < len(SpeciesNames) else str(Index)}:\n') # Header
      with Entry.open('rb') as PersonalRAW:
          LEARN_CNT = 0
          while True:
              Mv, Lvl = unpack("<HH", PersonalRAW.read(4))
              if Lvl == 0xFFFF and Mv == 0xFFFF:
                PERSONAL_ENTRY.write(f'  - LEARNSET_END:\n')
                PERSONAL_ENTRY.write(f'    - MOVE: 0xFFFF\n')
                PERSONAL_ENTRY.write(f'    - LEVEL: 0xFFFF\n')
                break
              
              PERSONAL_ENTRY.write(f'  - LEARNSET_ENTRY_{LEARN_CNT}:\n')
              PERSONAL_ENTRY.write(f'    - MOVE: MOVE_{MoveNames[Mv]}\n')
              PERSONAL_ENTRY.write(f'    - LEVEL: {Lvl}\n')
              LEARN_CNT += 1
      Index += 1
