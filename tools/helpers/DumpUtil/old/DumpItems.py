from struct import unpack
from pathlib import Path


PersonalExt = Path('items')
Names = []
with open('txtdmp/Items.txt', 'r') as NamesRAW:
  UNKCOUNT = 0
  while True:
    Name = NamesRAW.readline()
    if Name != "":
      Names.append(Name.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])
      if '???' in Names[-1]:
        Names[-1] = Names[-1].replace('???', f'UNKNOWN_{UNKCOUNT}')
        UNKCOUNT += 1
    else:
      break
Count = 0
with open('include/Items.h', 'w') as Personal:
  Personal.write(f'#ifndef __ITEMS_H\n#define __ITEMS_H\n\n')
  Personal.write(f'#include \"swan/swantypes.h\"\n\n')
  for Entry in sorted(PersonalExt.glob('*')):
      Personal.write(f'#define ITEM_{Names[Count]} {Count}\n')
      Count += 1
  Personal.write('''
                 
typedef struct
{
  u8 CureInflict;
  u8 Boost[4];
  u8 FunctionFlags0;
  u8 FunctionFlags1;
  char EVHP;
  char EVATK;
  char EVDEF;
  char EVSPE;
  char EVSPA;
  char EVSPD;
  u8 HealAmount;
  u8 PPGain;
  char Friendship1;
  char Friendship2;
  char Friendship3;
  char field_1F;
  char field_20;
} ItemBattleStats;
                 
typedef struct
{
  u16 Price;
  u8 HeldEffect;
  u8 HeldArgument;
  u8 NaturalGiftEffect;
  u8 FlingEffect;
  u8 FlingPower;
  u8 NaturalGiftPower;
  u16 Packed;
  u8 EffectField;
  u8 EffectBattle;
  u8 HasBattleStats;
  u8 ItemClass;
  u8 Consumable;
  u8 SortIndex;
  ItemBattleStats BattleStats;
} ITEM_DATA;

''')
  Personal.write(f'#endif\n')
Count = 0
with open('src/arc/pml/Items.c', 'w') as Personal:
  Personal.write(f'#include "Items.h"\n\nu32 __size = sizeof(ITEM_DATA);\n\nconst ITEM_DATA __data[] = {{\n')
  for Entry in sorted(PersonalExt.glob('*')):
    Personal.write(f'\t[ITEM_{Names[Count]}] = {{\n') # Header
    # Write EntryData
    with Entry.open('rb') as PersonalRAW:
        Personal.write(f'\t\t.Price = {unpack("<H", PersonalRAW.read(2))[0]},\n')
        Personal.write(f'\t\t.HeldEffect = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.HeldArgument = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.NaturalGiftEffect = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.FlingEffect = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.FlingPower = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.NaturalGiftPower = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.Packed = {unpack("<H", PersonalRAW.read(2))[0]},\n')
        Personal.write(f'\t\t.EffectField = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.EffectBattle = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.HasBattleStats = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.ItemClass = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.Consumable = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.SortIndex = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.BattleStats = {{\n')
        Personal.write(f'\t\t\t.CureInflict = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.Boost = {{\n')
        for x in range(4):
          Personal.write(f'\t\t\t\t{unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t}},\n')
        Personal.write(f'\t\t\t.FunctionFlags0 = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.FunctionFlags1 = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.EVHP = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.EVATK = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.EVDEF = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.EVSPE = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.EVSPA = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.EVSPD = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.HealAmount = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.PPGain = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.Friendship1 = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.Friendship2 = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.Friendship3 = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.field_1F = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t\t.field_20 = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t}}\n')
        Personal.write(f'\t}},\n')
        PersonalRAW.close()
    Count += 1
  Personal.write(f'}};')
  Personal.close()
