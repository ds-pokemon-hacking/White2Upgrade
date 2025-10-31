from struct import unpack
from pathlib import Path
'''

'''

Count = 0
PersonalExt = Path('moves')

MoveNames = []
SplitNames = ["SPLIT_STATUS", "SPLIT_PHYSICAL", "SPLIT_SPECIAL"]
TypeNames = ["TYPE_NORMAL", "TYPE_FIGHTING", "TYPE_FLYING", "TYPE_POISON",
            "TYPE_GROUND", "TYPE_ROCK", "TYPE_BUG", "TYPE_GHOST", "TYPE_STEEL", 
            "TYPE_FIRE", "TYPE_WATER", "TYPE_GRASS", "TYPE_ELECTRIC", "TYPE_PSYCHIC", "TYPE_ICE", "TYPE_DRAGON", "TYPE_DARK", "TYPE_FAIRY"]
QualityNames = ["EFFECT_NO_SPECIAL_EFFECT", "EFFECT_STATUS_INFLICTING","EFFECT_TARGET_STAT_CHANGING","EFFECT_HEALING","EFFECT_CHANCE_TO_INFLICT_STATUS","EFFECT_RAISING_TARGET_STAT", 
                "EFFECT_DAMAGE_AND_TARGET_STAT_EFFECT","EFFECT_DAMAGE_AND_USER_STAT_EFFECT","EFFECT_LIFE_STEAL","EFFECT_OHKO","EFFECT_FIELD_EFFCT","EFFECT_SIDE_EFFCT", "EFFECT_FORCE_SWITCH_OUT", "EFFECT_OTHERS"]
StatusNames = ["STATUS_NONE", "STATUS_PARALYZE", "STATUS_SLEEP", "STATUS_FREEZE", "STATUS_BURN", "STATUS_POSION", "STATUS_CONFUSE", "STATUS_INFATUATE", "STATUS_BINDING", 
               "STATUS_NIGHTMARE", "STATUS_CURSED", "STATUS_TAUNT", "STATUS_TORMENT", "STATUS_DISABLE_LAST_MOVE", "STATUS_NEXT_TURN_SLEEP", "STATUS_HEAL_BLOCK", "STATUS_DISBALE_ABILITY", "STATUS_FORESIGHT", 
               "STATUS_LEECH_SEED", "STATUS_BLOCK_ITEM", "STATUS_PERISH_SONG", "STATUS_INGRAIN"]

Duration = ["INFLICT_DURATION_NONE", "INFLICT_DURATION_PERMANENT", "INFLICT_DURATION_TRUN",
            "INFLICT_DURATION_POKE", "INFLICT_DURATION_POKE_TURN"]
Stats = ["0", "STAT_ATK", "STAT_DEF", "STAT_SPATK", "STAT_SPDEF", "STAT_SPEED", "STAT_ACCURACY", "STAT_EVASION", "STAT_ALL"]
FlagTable = ["FLAG_CONTACT","FLAG_REQUIRES_CHARGE","FLAG_RECHARGE_TURN","FLAG_BLOCKED_BY_PROTECT","FLAG_REFLECTED_BY_MAGIC_COAT","FLAG_STOLEN_BY_SNATCH",
             "FLAG_COPIED_BY_MIRROR_MOVE","FLAG_PUNCH_MOVE","FLAG_SOUND_MOVE","FLAG_GROUNDED_BY_GRAVITY","FLAG_DEFROSTS_TARGETS","FLAG_HITS_NON_ADJACENT_OPPONENTS",
             "FLAG_HEALING_MOVE","FLAG_HITS_THROUGH_SUBSTITUTE"]

Target = ["TARGET_OTHER_SELECT","TARGET_FRIEND_AND_USER","TARGET_FRIEND_SELECT","TARGET_ENEMY_SELECT","TARGET_OTHER_ALL","TARGET_ENEMY_ALL","TARGET_FRIEND_ALL","TARGET_USER","TARGET_ALL",
           "TARGET_ENEMY_RANDOM","TARGET_FIELD","TARGET_FIELD_SIDE_ENEMY","TARGET_FIELD_SIDE_FRIEND","TARGET_UNKNOWN"]

with open('txtdmp/Moves.txt') as Moves:
    while (CurrMove := Moves.readline()) != '':
        MoveNames.append(CurrMove.upper().replace('\X2019', '').replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

with open('include/Moves.h', 'w') as Personal:
  Personal.write(f'#ifndef __MOVES_H\n#define __MOVES_H\n\n')
  Personal.write(f'#include "swan/swantypes.h"\n')
  Personal.write('''
#define MUST_HIT   101

#define INFLICT_DURATION_NONE        0
#define INFLICT_DURATION_PERMANENT   1 // permanent status
#define INFLICT_DURATION_TRUN        2 // minturn ~ maxturn status
#define INFLICT_DURATION_POKE        3
#define INFLICT_DURATION_POKE_TURN   4

#define STAT_NONE       0
#define STAT_ATK        1
#define STAT_DEF        2
#define STAT_SPATK      3
#define STAT_SPDEF      4
#define STAT_SPEED      5
#define STAT_ACCURACY   6
#define STAT_EVASION    7
#define STAT_ALL        8

#define SPLIT_STATUS    0x0
#define SPLIT_PHYSICAL  0x1
#define SPLIT_SPECIAL   0x2

#define TYPE_NORMAL           0
#define TYPE_FIGHTING         1
#define TYPE_FLYING           2
#define TYPE_POISON           3
#define TYPE_GROUND           4
#define TYPE_ROCK             5
#define TYPE_BUG              6
#define TYPE_GHOST            7
#define TYPE_STEEL            8
#define TYPE_FIRE             9
#define TYPE_WATER            10
#define TYPE_GRASS            11
#define TYPE_ELECTRIC         12
#define TYPE_PSYCHIC          13
#define TYPE_ICE              14
#define TYPE_DRAGON           15
#define TYPE_DARK             16
#define TYPE_FAIRY          17

#define EFFECT_NO_SPECIAL_EFFECT              0
#define EFFECT_STATUS_INFLICTING              1
#define EFFECT_TARGET_STAT_CHANGING           2
#define EFFECT_HEALING                        3
#define EFFECT_CHANCE_TO_INFLICT_STATUS       4
#define EFFECT_RAISING_TARGET_STAT            5
#define EFFECT_DAMAGE_AND_TARGET_STAT_EFFECT  6
#define EFFECT_DAMAGE_AND_USER_STAT_EFFECT    7
#define EFFECT_LIFE_STEAL                     8
#define EFFECT_OHKO                           9
#define EFFECT_FIELD_EFFCT                    10
#define EFFECT_SIDE_EFFCT                     11
#define EFFECT_FORCE_SWITCH_OUT               12
#define EFFECT_OTHERS                         13

#define FLAG_CONTACT                            (1 << 0)
#define FLAG_REQUIRES_CHARGE                    (1 << 1)
#define FLAG_RECHARGE_TURN                      (1 << 2)
#define FLAG_BLOCKED_BY_PROTECT                 (1 << 3)
#define FLAG_REFLECTED_BY_MAGIC_COAT            (1 << 4)
#define FLAG_STOLEN_BY_SNATCH                   (1 << 5)
#define FLAG_COPIED_BY_MIRROR_MOVE              (1 << 6)
#define FLAG_PUNCH_MOVE                         (1 << 7)
#define FLAG_SOUND_MOVE                         (1 << 8)
#define FLAG_GROUNDED_BY_GRAVITY                (1 << 9)
#define FLAG_DEFROSTS_TARGETS                   (1 << 10)
#define FLAG_HITS_NON_ADJACENT_OPPONENTS        (1 << 11)
#define FLAG_HEALING_MOVE                       (1 << 12)
#define FLAG_HITS_THROUGH_SUBSTITUTE            (1 << 13)

#define TARGET_OTHER_SELECT         0
#define TARGET_FRIEND_AND_USER      1
#define TARGET_FRIEND_SELECT        2
#define TARGET_ENEMY_SELECT         3
#define TARGET_OTHER_ALL            4
#define TARGET_ENEMY_ALL            5
#define TARGET_FRIEND_ALL           6
#define TARGET_USER                 7
#define TARGET_ALL                  8
#define TARGET_ENEMY_RANDOM         9
#define TARGET_FIELD                10
#define TARGET_FIELD_SIDE_ENEMY     11
#define TARGET_FIELD_SIDE_FRIEND    12
#define TARGET_UNKNOWN              13

#define STATUS_NONE                 0
#define STATUS_PARALYZE             1
#define STATUS_SLEEP                2
#define STATUS_FREEZE               3
#define STATUS_BURN                 4
#define STATUS_POSION               5
#define STATUS_CONFUSE              6
#define STATUS_INFATUATE            7
#define STATUS_BINDING              8
#define STATUS_NIGHTMARE            9
#define STATUS_CURSED               10
#define STATUS_TAUNT                11
#define STATUS_TORMENT              12
#define STATUS_DISABLE_LAST_MOVE    13
#define STATUS_NEXT_TURN_SLEEP      14
#define STATUS_HEAL_BLOCK           15
#define STATUS_DISBALE_ABILITY      16
#define STATUS_FORESIGHT            17
#define STATUS_LEECH_SEED           18
#define STATUS_BLOCK_ITEM           19
#define STATUS_PERISH_SONG          20
#define STATUS_INGRAIN              21

// did't test
#define STATUS_CANT_ESCAPE          22
#define STATUS_ENCORE               23

#define STATUS_SPECIAL_CODE         0xFFFF

typedef struct {
  u8 Type;
  u8 Quality;
  u8 Category;
  u8 Power;
  u8 Accuracy;
  u8 BasePP;
  char Priority;
  u8 HitMin :4;
  u8 HitMax :4;
  u16 InflictStatus;
  u8 InflictChance;
  u8 InflictDuration;
  u8 TurnMin;
  u8 TurnMax;
  u8 CritStage;
  u8 FlinchRate;
  u16 WazaSeqID;
  char Recoil;
  char Heal;
  u8 Target;
  char StatChangeStats[3];
  char StatChangeStages[3];
  char StatChangeChances[3];
  // u16 pad;
  int Flags;
} MOVE_DATA;
\n''')
  for x in range(len(MoveNames)):
    Personal.write(f'#define MOVE_{MoveNames[x]} {x}\n')
  Personal.write(f'\n#endif\n')

with open(f'src/arc/pml/Moves.c', 'w') as Personal:
  Personal.write(f'#include "Moves.h"\n\nu32 __size = sizeof(MOVE_DATA);\n\nconst MOVE_DATA __data[] = {{\n')
  for Entry in sorted(PersonalExt.glob('*')):
    Personal.write(f'\t[MOVE_{MoveNames[Count]}] = {{\n') # Header
    # Write EntryData
    with Entry.open('rb') as PersonalRAW:
        Personal.write(f'\t\t.Type = {TypeNames[unpack("B", PersonalRAW.read(1))[0]]},\n')
        Personal.write(f'\t\t.Quality = {QualityNames[unpack("B", PersonalRAW.read(1))[0]]},\n')
        Personal.write(f'\t\t.Category = {SplitNames[unpack("B", PersonalRAW.read(1))[0]]},\n')
        Personal.write(f'\t\t.Power = {unpack("B", PersonalRAW.read(1))[0]},\n')
        acc = str(unpack("B", PersonalRAW.read(1))[0]).replace("101","MUST_HIT")
        Personal.write(f'\t\t.Accuracy = {acc},\n')
        Personal.write(f'\t\t.BasePP = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.Priority = {unpack("b", PersonalRAW.read(1))[0]},\n')
        HitMinMax = unpack("B", PersonalRAW.read(1))[0]
        Personal.write(f'\t\t.HitMin = {HitMinMax & 0xF},\n')
        Personal.write(f'\t\t.HitMax = {HitMinMax >> 4},\n')
        status = unpack("<H", PersonalRAW.read(2))[0]
        if status == 0xFFFF:
          InflictStatus = "STATUS_SPECIAL_CODE"
        else:
          InflictStatus = StatusNames[status]
        Personal.write(f'\t\t.InflictStatus = {InflictStatus},\n')
        Personal.write(f'\t\t.InflictChance = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.InflictDuration = {Duration[unpack("B", PersonalRAW.read(1))[0]]},\n')
        Personal.write(f'\t\t.TurnMin = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.TurnMax = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.CritStage = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.FlinchRate = {unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.WazaSeqID = {unpack("<H", PersonalRAW.read(2))[0]},\n')
        Personal.write(f'\t\t.Recoil = {unpack("b", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.Heal = {unpack("b", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t.Target = {Target[unpack("B", PersonalRAW.read(1))[0]]},\n')
        Personal.write(f'\t\t.StatChangeStats = {{\n')
        for x in range(3):
          Personal.write(f'\t\t\t{Stats[unpack("B", PersonalRAW.read(1))[0]]},\n')
        Personal.write(f'\t\t}},\n')

        Personal.write(f'\t\t.StatChangeStages = {{\n')
        for x in range(3):
          Personal.write(f'\t\t\t{unpack("b", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t}},\n')

        Personal.write(f'\t\t.StatChangeChances = {{\n')
        for x in range(3):
          Personal.write(f'\t\t\t{unpack("B", PersonalRAW.read(1))[0]},\n')
        Personal.write(f'\t\t}},\n')

        pad = unpack("H", PersonalRAW.read(2))[0]
        # Personal.write(f'\t\t.padding = {pad},\n')

        flag = unpack("<L", PersonalRAW.read(4))[0]
        flagString = ""
        if flag != 0:
          # for i in range(32):
          #   if (flag >> i) & 1:
          #     print(i)
          #     flagString += f"{FlagTable[i]} | "
          Personal.write(f'\t\t.Flags = {flag},\n')
        else:
          Personal.write(f'\t\t.Flags = 0,\n')
        PersonalRAW.close()
    Personal.write(f'\t}},\n')
    Count += 1
  Personal.write(f'}};\n')
  Personal.close()