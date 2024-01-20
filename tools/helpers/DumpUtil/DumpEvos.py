from struct import unpack
from pathlib import Path

PersonalExt = Path('evolutions')

MoveNames = []
SpeciesNames = []
ItemNames = []
MethodNames = []

with open('../txtdmp/Moves.txt') as Moves:
    while (CurrMove := Moves.readline()) != '':
        MoveNames.append(CurrMove.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('../txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('../txtdmp/Items.txt') as Items:
    while (CurrItems := Items.readline()) != '':
        ItemNames.append(CurrItems.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('../txtdmp/EvoMethods.txt') as Methods:
    while (CurrMethods := Methods.readline()) != '':
        MethodNames.append(CurrMethods.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

Count = 0
ExpandedLimit = 0
#Dump original files and skip the expanded files for alternate forms
for x in PersonalExt.glob('*'):
    with x.open('rb') as EVO_RAW:
        if (Count == 650):
            ExpandedLimit = 72
            Count += ExpandedLimit
        with open(f'../../../data/pml/evolutions/{Count}.yml', 'w') as EVO_TXT:
            EVO_TXT.write(f'SPECIES_{SpeciesNames[Count] if Count < len(SpeciesNames) else Count}:\n') # Header
            Count += 1
            EVO_CNT = 0
            while EVO_RAW.tell() < x.stat().st_size:
                EVO_TXT.write(f'  - EVOLUTION_{EVO_CNT}:\n')
                Method_Number = unpack("<H", EVO_RAW.read(2))[0]
                EVO_TXT.write(f'    - Method: METHOD_{MethodNames[Method_Number] if Method_Number < len(MethodNames) else Method_Number} \n')

                
                #Write the parameter value according to the evolution method
                Parameter = unpack("<H", EVO_RAW.read(2))[0]
                match Method_Number:
                    case 6 | 8 | 17 | 18 | 19 | 20:
                        EVO_TXT.write(f'    - Parameter: ITEM_{ItemNames[Parameter] if Parameter < len(ItemNames) else Parameter} \n')
                    case 21:
                        EVO_TXT.write(f'    - Parameter: MOVE_{MoveNames[Parameter] if Parameter < len(MoveNames) else Parameter} \n')
                    case 7 | 22:
                        EVO_TXT.write(f'    - Parameter: SPECIES_{SpeciesNames[Parameter] if Parameter < len(SpeciesNames) else Parameter} \n')
                    case _ :
                        EVO_TXT.write(f'    - Parameter: {Parameter} \n')
                        
                TargetSpeciesNum = unpack("<H", EVO_RAW.read(2))[0]
                EVO_TXT.write(f'    - Target Species: SPECIES_{SpeciesNames[TargetSpeciesNum] if TargetSpeciesNum < len(SpeciesNames) else TargetSpeciesNum} \n')
                EVO_CNT += 1
            #8th Evolution Slot
            EVO_TXT.write(f'  - EVOLUTION_{7}:\n')
            EVO_TXT.write(f'    - Method: METHOD_NONE \n')
            EVO_TXT.write(f'    - Parameter: 0 \n')
            EVO_TXT.write(f'    - Target Species: SPECIES_NONE \n')