from pathlib import Path
from struct import unpack
'''
struct PersonalData {
  u8 BaseHP;
  u8 BaseATK;
  u8 BaseDEF;
  u8 BaseSPE;
  u8 BaseSPA;
  u8 BaseSPD;
  u8 Type1;
  u8 Type2;
  u8 CaptureRate;
  u8 EvoStage;
  u16 EVYield;
  u16 WildItem50;
  u16 WildItem5;
  u16 WildItem1;
  u8 GenderProb;
  u8 EggHappiness;
  u8 BaseHappiness;
  u8 ExpGroup;
  u8 EggGroup1;
  u8 EggGroup2;
  u8 Abil1;
  u8 Abil2;
  u8 AbilHidden;
  u8 EscapeRate;
  u16 FormeDataOffs;
  u16 FormeSpriteOffs;
  u8 FormeCount;
  u8 Color;
  u16 BaseEXP;
  u16 HeightCm;
  u16 WeightCg;
  int TMHM1;
  int TMHM2;
  int TMHM3;
  int TMHM4;
  int TypeTutors;
  int SpecialTutors[4];
};
'''
Count = 0
PersonalExt = Path('personal')
SpeciesNames = []
TypeNames = []

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('txtdmp/Types.txt') as Types:
    while (CurrType := Types.readline()) != '':
        TypeNames.append(CurrType.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

print(TypeNames)
Path('personal_txt').mkdir(exist_ok=True)

for Entry in sorted(PersonalExt.glob('*')):
    with open(f'personal_txt/{Entry.stem[-3:]}.yml', 'w') as PERSONAL_ENTRY:
        PERSONAL_ENTRY.write(f'SPECIES_{SpeciesNames[Count] if Count < len(SpeciesNames) else Count}:\n') # Header
        Count += 1
        # Write EntryData
        with Entry.open('rb') as PersonalRAW:
            PERSONAL_ENTRY.write(f'- Base HP: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Attack: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Defense: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Speed: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Special Attack: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Special Defense: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Primary Type: {TypeNames[unpack("B", PersonalRAW.read(1))[0]]}\n')
            PERSONAL_ENTRY.write(f'- Secondary Type: {TypeNames[unpack("B", PersonalRAW.read(1))[0]]}\n')
            PERSONAL_ENTRY.write(f'- Capture Rate: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Evolution Stage: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- EV Yield: {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Wild Item (50%): {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Wild Item (5%): {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Wild Item (1%): {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Gender Probability: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Egg Happiness: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Happiness: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Experience Group: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Egg Group 1: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Egg Group 2: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Primary Ability : {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Secondary Ability: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Hidden Ability: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Escape Rate: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Form Data Offset: {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Form Sprite Offset: {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Form Count: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Color: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'- Base Experience: {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Height (cm): {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- Weight (cg): {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'- TM HM 1: {unpack("<i", PersonalRAW.read(4))[0]}\n')
            PERSONAL_ENTRY.write(f'- TM HM 2: {unpack("<i", PersonalRAW.read(4))[0]}\n')
            PERSONAL_ENTRY.write(f'- TM HM 3: {unpack("<i", PersonalRAW.read(4))[0]}\n')
            PERSONAL_ENTRY.write(f'- TM HM 4: {unpack("<i", PersonalRAW.read(4))[0]}\n')
            PERSONAL_ENTRY.write(f'- Type Tutors: {unpack("<i", PersonalRAW.read(4))[0]}\n')
            PERSONAL_ENTRY.write(f'- Special Tutors: \n')
            for x in range(4):
                PERSONAL_ENTRY.write(f'  - {unpack("<i", PersonalRAW.read(4))[0]}\n')
            PersonalRAW.close()
    PERSONAL_ENTRY.close()
