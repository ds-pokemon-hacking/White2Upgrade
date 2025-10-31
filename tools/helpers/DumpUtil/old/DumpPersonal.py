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

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('src/arc/pml/Personal.c', 'w') as Personal:
    Personal.write('#include "Species.h"\n#include "Personal.h"\nu32 __size = sizeof(PERSONAL_DATA);\n\n')
    Personal.write('const PERSONAL_DATA __data[] = {\n')
    for Entry in sorted(PersonalExt.glob('*')):
        Personal.write(f'\t[SPECIES_{SpeciesNames[Count] if Count < len(SpeciesNames) else Count}] = {{\n') # Header
        Count += 1
        # Write EntryData
        with Entry.open('rb') as PersonalRAW:
            Personal.write(f'\t\t.BaseHP = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseATK = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseDEF = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseSPE = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseSPA = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseSPD = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.Type1 = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.Type2 = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.CaptureRate = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.EvoStage = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.EVYield = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.WildItem50 = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.WildItem5 = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.WildItem1 = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.GenderProb = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.EggHappiness = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseHappiness = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.ExpGroup = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.EggGroup1 = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.EggGroup2 = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.Abil1 = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.Abil2 = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.AbilHidden = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.EscapeRate = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.FormeDataOffs = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.FormeSpriteOffs = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.FormeCount = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.Color = {unpack("B", PersonalRAW.read(1))[0]},\n')
            Personal.write(f'\t\t.BaseEXP = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.HeightCm = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.WeightCg = {unpack("<H", PersonalRAW.read(2))[0]},\n')
            Personal.write(f'\t\t.TMHM1 = {unpack("<L", PersonalRAW.read(4))[0]},\n')
            Personal.write(f'\t\t.TMHM2 = {unpack("<L", PersonalRAW.read(4))[0]},\n')
            Personal.write(f'\t\t.TMHM3 = {unpack("<L", PersonalRAW.read(4))[0]},\n')
            Personal.write(f'\t\t.TMHM4 = {unpack("<L", PersonalRAW.read(4))[0]},\n')
            Personal.write(f'\t\t.TypeTutors = {unpack("<L", PersonalRAW.read(4))[0]},\n')
            Personal.write(f'\t\t.SpecialTutors = {{\n')
            for x in range(4):
                Personal.write(f'\t\t\t{unpack("<L", PersonalRAW.read(4))[0]},\n')
            Personal.write(f'\t\t}},\n')
            PersonalRAW.close()
        Personal.write(f'\t}},\n')
    Personal.write(f'}};\n')
    Personal.close()
