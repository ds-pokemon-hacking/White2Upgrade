from pathlib import Path
from struct import unpack

SpeciesNames = []
ChildrenBinary = Path('children')
ChildrensTXT = Path('../../../data/pml/children')
with open('../txtdmp/Species.txt') as SpeciesFile:
    while (CurrSpecies := SpeciesFile.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])
    pass
#Dump original files, add alternate forms after skipping expanded files
i = 0
ExpandedLimit = 0
for x in ChildrenBinary.glob('*'):
    with x.open('rb') as Children_RAW:
        if (i > 649):
            ExpandedLimit = 72
        with (ChildrensTXT / f'{i + ExpandedLimit}.yml').open('w') as Children_TXT:
            Species = i + ExpandedLimit
            Children_TXT.write(f'SPECIES_{SpeciesNames[Species] if Species < len(SpeciesNames) else Species}:\n')
            Species = unpack("<H", Children_RAW.read(2))[0] + ExpandedLimit
            Children_TXT.write(f'  Child: {SpeciesNames[Species] if Species < len(SpeciesNames) else Species}\n')
            i += 1


