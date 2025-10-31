from struct import unpack
from pathlib import Path


PersonalExt = Path('evolutions')

MoveNames = []
SpeciesNames = []

with open('txtdmp/Moves.txt') as Moves:
    while (CurrMove := Moves.readline()) != '':
        MoveNames.append(CurrMove.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

# with open('include/Evolutions.h', 'w') as Personal:
#   Personal.write(f'#ifndef __EVOLUTIONS_H\n#define __EVOLUTIONS_H\n\n')
#   Personal.write(f'#include "swan/swantypes.h"\n')
#   Personal.write('''
# typedef struct {
#     uint16_t Method;
#     uint16_t Parameter;
#     uint16_t TargetSpecies;
# } EvoEntry;

# typedef struct {
#     EvoEntry Entries[0x7];
# } EVOLUTION_DATA;
# \n''')
#   Personal.write(f'\n#endif\n')

Count = 0
for Entry in sorted(PersonalExt.glob('*'), key=lambda x: int(x.stem[15:])):
    print(Entry)
    with open(f'evo_txt/{Entry.stem[-3:]}.yml', 'w') as PERSONAL_ENTRY:
        PERSONAL_ENTRY.write(f'SPECIES_{SpeciesNames[Count] if Count < len(SpeciesNames) else Count}:\n') # Header
        Count += 1
        with Entry.open('rb') as RAW:
            EVO_CNT = 0
            while RAW.tell() < Entry.stat().st_size:
                PERSONAL_ENTRY.write(f'  - EVOLUTION_{EVO_CNT}:\n')
                PERSONAL_ENTRY.write(f'    - Method: {unpack("<H", RAW.read(2))[0]} \n')
                PERSONAL_ENTRY.write(f'    - Parameter: {unpack("<H", RAW.read(2))[0]} \n')
                TargetSpeciesNum = unpack("<H", RAW.read(2))[0]
                PERSONAL_ENTRY.write(f'    - Target Species: SPECIES_{SpeciesNames[TargetSpeciesNum] if TargetSpeciesNum < len(SpeciesNames) else TargetSpeciesNum} \n')
                EVO_CNT += 1