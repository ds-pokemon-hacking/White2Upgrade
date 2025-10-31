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

with open('include/Evolutions.h', 'w') as Personal:
  Personal.write(f'#ifndef __EVOLUTIONS_H\n#define __EVOLUTIONS_H\n\n')
  Personal.write(f'#include "swan/swantypes.h"\n')
  Personal.write('''
typedef struct {
    uint16_t Method;
    uint16_t Parameter;
    uint16_t TargetSpecies;
} EvoEntry;

typedef struct {
    EvoEntry Entries[0x7];
} EVOLUTION_DATA;
\n''')
  Personal.write(f'\n#endif\n')

Count = 0
with open('src/arc/pml/Evolutions.c', 'w') as Personal:
    Personal.write(f'#include "Species.h"\n#include "Evolutions.h"\n\nu32 __size = sizeof(EVOLUTION_DATA);\n\nconst EVOLUTION_DATA __data[] = {{\n')
    for Entry in sorted(PersonalExt.glob('*'), key=lambda x: int(x.stem[15:])):
        print(Entry)
        Personal.write(f'\t[SPECIES_{SpeciesNames[Count] if Count < len(SpeciesNames) else str(Count)}] = {{\n') # Header
        with Entry.open('rb') as PersonalRAW:
            Personal.write(f'\t\t .Entries = {{\n')
            for x in range(7):
                Personal.write(f'\t\t\t{{\n')
                Method, Param, Target = unpack("<HHH", PersonalRAW.read(6))
                Personal.write(f'\t\t\t\t.Method = {Method},\n')
                Personal.write(f'\t\t\t\t.Parameter = {Param},\n')
                Personal.write(f'\t\t\t\t.TargetSpecies = SPECIES_{SpeciesNames[Target] if Target < len(SpeciesNames) else Target},\n')
                Personal.write(f'\t\t\t}},\n')   
            Personal.write(f'\t\t }},\n')
            PersonalRAW.close()
        Personal.write(f'\t}},\n')
        Count += 1
    Personal.write(f'}};\n')
    Personal.close()