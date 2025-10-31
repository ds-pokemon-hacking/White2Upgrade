from struct import unpack
from pathlib import Path
'''
typedef struct {
    u16 ID;
} CHILD_DATA;
'''

PersonalExt = Path('children')

SpeciesNames = []

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('\X2019', '').replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('include/Children.h', 'w') as Personal:
  Personal.write(f'#ifndef __CHILD_H\n#define __CHILD_H\n\n')
  Personal.write(f'#include "swan/swantypes.h"\n')
  Personal.write('''
typedef struct {
    u16 ID;
} CHILD_DATA;
\n''')
  Personal.write(f'\n#endif\n')

Count = 0
with open(f'src/arc/pml/Children.c', 'w') as Personal:
  Personal.write(f'#include "Species.h"\n#include "Children.h"\n\nu32 __size = sizeof(CHILD_DATA);\n\nconst CHILD_DATA __data[] = {{\n')
  for Entry in sorted(PersonalExt.glob('*'), key=lambda x: int(x.stem[13:])):
    Personal.write(f'\t[SPECIES_{SpeciesNames[Count]}] = {{\n') # Header
    # Write EntryData
    with Entry.open('rb') as PersonalRAW:
        Personal.write(f'\t\t.ID = {unpack("<H", PersonalRAW.read(2))[0]},\n')
        PersonalRAW.close()
    Personal.write(f'\t}},\n')
    Count += 1
  Personal.write(f'}};\n')
  Personal.close()
