# SpeciesNames = []

# with open('txtdmp/Species.txt') as Species:
#     while (CurrSpecies := Species.readline()) != '':
#         SpeciesNames.append(CurrSpecies.upper().replace('\X2019', '').replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

# with open('include/Species.h', 'w') as SpeciesH:
#     SpeciesH.write('''
# #ifndef __SPECIES_H
# #define __SPECIES_H\n
# ''')
#     for x in range(827):
#         SpeciesH.write(f'#define SPECIES_{SpeciesNames[x] if x in range(len(SpeciesNames)) else x} {x}\n')
#     SpeciesH.write('''
# #endif
#     ''')

SpeciesNames = []

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('\X2019', '').replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('species.yml', 'w') as SpeciesH:
    for x in range(827):
        SpeciesH.write(f'- SPECIES_{SpeciesNames[x] if x in range(len(SpeciesNames)) else x}: {x}\n')