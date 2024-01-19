from struct import unpack
from pathlib import Path

PersonalExt = Path('evolutions')

MoveNames = []
SpeciesNames = []

with open('../txtdmp/Moves.txt') as Moves:
    while (CurrMove := Moves.readline()) != '':
        MoveNames.append(CurrMove.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('../txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

Count = 0

for x in PersonalExt.glob('*'):
    with x.open('rb') as EVO_RAW:
        with open(f'../../../data/pml/evolutions/{Count}.yml', 'w') as EVO_TXT:
            EVO_TXT.write(f'SPECIES_{SpeciesNames[Count] if Count < len(SpeciesNames) else Count}:\n') # Header
            Count += 1
            EVO_CNT = 0
            while EVO_RAW.tell() < x.stat().st_size:
                EVO_TXT.write(f'  - EVOLUTION_{EVO_CNT}:\n')
                EVO_TXT.write(f'    - Method: {unpack("<H", EVO_RAW.read(2))[0]} \n')
                EVO_TXT.write(f'    - Parameter: {unpack("<H", EVO_RAW.read(2))[0]} \n')
                TargetSpeciesNum = unpack("<H", EVO_RAW.read(2))[0]
                EVO_TXT.write(f'    - Target Species: SPECIES_{SpeciesNames[TargetSpeciesNum] if TargetSpeciesNum < len(SpeciesNames) else TargetSpeciesNum} \n')
                EVO_CNT += 1
            #8th Evolution Slot
            EVO_TXT.write(f'  - EVOLUTION_{7}:\n')
            EVO_TXT.write(f'    - Method: 0 \n')
            EVO_TXT.write(f'    - Parameter: 0 \n')
            EVO_TXT.write(f'    - Target Species: SPECIES_NONE \n')