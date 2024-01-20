import os
from pathlib import Path
from struct import unpack
#trdata: a091 trpoke: a092
SpeciesNames, ItemNames, MoveNames = [], [], []
TrdataDIR = Path('trdata')
TrpokeDIR = Path('trpoke')
TrainersTXT = Path('../../../data/trainers')
TrainerID = 0
with open('../txtdmp/Species.txt') as SpeciesFile:
    while (CurrSpecies := SpeciesFile.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])
    pass
with open('../txtdmp/Items.txt') as ItemsFile:
    while (CurrItems := ItemsFile.readline()) != '':
        ItemNames.append(CurrItems.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])
    pass
with open('../txtdmp/Moves.txt') as MovesFile:
    while (CurrMoves := MovesFile.readline()) != '':
        MoveNames.append(CurrMoves.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])
    pass
#Get all trpoke files in a list for later access
TrPokeFiles = os.listdir(TrpokeDIR)

#Iterate trough all trdata files
for x in TrdataDIR.glob('*'):
    if (TrainerID == 0): #Skip File 0 as it's a dummy
         TrainerID += 1
         continue
    with x.open('rb') as TRDATA_RAW:
        #Parse trdata first
        with (TrainersTXT / f'{x.stem}.yml').open('w') as Trainer_TXT:

                Flags = unpack("B", TRDATA_RAW.read(1))[0]
                Trainer_TXT.write(f'CLASS: {unpack("B", TRDATA_RAW.read(1))[0]}\n')
                Trainer_TXT.write(f'BATTLE_TYPE: {unpack("B", TRDATA_RAW.read(1))[0]}\n')
                PokeCount = unpack("B", TRDATA_RAW.read(1))[0]

                #Trainer Items
                Trainer_TXT.write(f'ITEMS: \n')
                for z in range(4):
                    Item = unpack("<H", TRDATA_RAW.read(2))[0]
                    Trainer_TXT.write(f'    - ITEM_{ItemNames[Item] if Item < len(ItemNames) else Item}\n')

                Trainer_TXT.write(f'AI: {unpack("<I", TRDATA_RAW.read(4))[0]}\n')
                Trainer_TXT.write(f'CAN_HEAL: {unpack("B", TRDATA_RAW.read(1))[0]}\n')
                Trainer_TXT.write(f'REWARD_MONEY: {unpack("B", TRDATA_RAW.read(1))[0]}\n')
                Item = unpack("<H", TRDATA_RAW.read(2))[0]
                Trainer_TXT.write(f'REWARD_ITEM: ITEM_{ItemNames[Item] if Item < len(ItemNames) else Item}\n')
                Trainer_TXT.write(f'PARTY:\n')

                #Parse trpoke
                TRPOKE_RAW = open(Path(str(TrpokeDIR) + '/' + TrPokeFiles[TrainerID]),'rb')
                for y in range(PokeCount):

                    DiffValue = unpack("B", TRPOKE_RAW.read(1))[0]
                    Abil_Gender_Value = unpack("B", TRPOKE_RAW.read(1))[0]
                    Ability = Abil_Gender_Value // 16
                    Gender = Abil_Gender_Value % 16
                    Level = unpack("<H", TRPOKE_RAW.read(2))[0]
                    Species = unpack("<H", TRPOKE_RAW.read(2))[0]
                    Form = unpack("<H", TRPOKE_RAW.read(2))[0]

                    Trainer_TXT.write(f'  - SPECIES: SPECIES_{SpeciesNames[Species] if Species < len(SpeciesNames) else Species}\n')
                    Trainer_TXT.write(f'    LEVEL: {Level}\n')
                    Trainer_TXT.write(f'    FORM: {Form}\n')
                    Trainer_TXT.write(f'    GENDER: {Gender}\n')
                    Trainer_TXT.write(f'    ABILITY: {Ability}\n')
                    Trainer_TXT.write(f'    DIFFICULTY_VALUE: {DiffValue}\n')

                    if (((Flags >> 0x1) & 0x1) == 1): #Write HeldItem field if Flag is set in trdata
                        Item = unpack("<H", TRPOKE_RAW.read(2))[0]
                        Trainer_TXT.write(f'    HELD_ITEM: ITEM_{ItemNames[Item] if Item < len(ItemNames) else Item}\n')
                    
                    MoveID = 0
                    if ((Flags & 0x1) == 1): #Write Moveset field if Flag is set in trdata
                        Trainer_TXT.write(f'    MOVES:\n')
                        for z in range(4):
                            MoveID = unpack("<H", TRPOKE_RAW.read(2))[0]
                            Trainer_TXT.write(f'        - MOVE_{MoveNames[MoveID] if MoveID < len(MoveNames) else MoveID}\n')
                        
    TrainerID += 1