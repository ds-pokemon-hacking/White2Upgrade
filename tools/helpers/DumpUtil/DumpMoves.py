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

with open('moves.yml', 'w') as Personal:
  Personal.write('''
DEFINE:
  - MUST_HIT: 101
                
  - INFLICT_DURATION_NONE: 0
  - INFLICT_DURATION_PERMANENT: 1 // permanent status
  - INFLICT_DURATION_TURN: 2 // minturn ~ maxturn status
  - INFLICT_DURATION_POKE: 3
  - INFLICT_DURATION_POKE_TURN: 4
                
  - STAT_NONE: 0
  - STAT_ATK: 1
  - STAT_DEF: 2
  - STAT_SPATK: 3
  - STAT_SPDEF: 4
  - STAT_SPEED: 5
  - STAT_ACCURACY: 6
  - STAT_EVASION: 7
  - STAT_ALL: 8
  - SPLIT_STATUS: 0x0
  - SPLIT_PHYSICAL: 0x1
  - SPLIT_SPECIAL: 0x2

  - EFFECT_NO_SPECIAL_EFFECT: 0
  - EFFECT_STATUS_INFLICTING: 1
  - EFFECT_TARGET_STAT_CHANGING: 2
  - EFFECT_HEALING: 3
  - EFFECT_CHANCE_TO_INFLICT_STATUS: 4
  - EFFECT_RAISING_TARGET_STAT: 5
  - EFFECT_DAMAGE_AND_TARGET_STAT_EFFECT: 6
  - EFFECT_DAMAGE_AND_USER_STAT_EFFECT: 7
  - EFFECT_LIFE_STEAL: 8
  - EFFECT_OHKO: 9
  - EFFECT_FIELD_EFFCT: 10
  - EFFECT_SIDE_EFFCT: 11
  - EFFECT_FORCE_SWITCH_OUT: 12
  - EFFECT_OTHERS: 13

  - FLAG_CONTACT: 1
  - FLAG_REQUIRES_CHARGE: 2
  - FLAG_RECHARGE_TURN: 4
  - FLAG_BLOCKED_BY_PROTECT: 8
  - FLAG_REFLECTED_BY_MAGIC_COAT: 16
  - FLAG_STOLEN_BY_SNATCH: 32
  - FLAG_COPIED_BY_MIRROR_MOVE: 64
  - FLAG_PUNCH_MOVE: 128
  - FLAG_SOUND_MOVE: 256
  - FLAG_GROUNDED_BY_GRAVITY: 512
  - FLAG_DEFROSTS_TARGETS: 1024
  - FLAG_HITS_NON_ADJACENT_OPPONENTS: 2048
  - FLAG_HEALING_MOVE: 4096
  - FLAG_HITS_THROUGH_SUBSTITUTE: 8192

  - TARGET_OTHER_SELECT: 0
  - TARGET_FRIEND_AND_USER: 1
  - TARGET_FRIEND_SELECT: 2
  - TARGET_ENEMY_SELECT: 3
  - TARGET_OTHER_ALL: 4
  - TARGET_ENEMY_ALL: 5
  - TARGET_FRIEND_ALL: 6
  - TARGET_USER: 7
  - TARGET_ALL: 8
  - TARGET_ENEMY_RANDOM: 9
  - TARGET_FIELD: 10
  - TARGET_FIELD_SIDE_ENEMY: 11
  - TARGET_FIELD_SIDE_FRIEND: 12
  - TARGET_UNKNOWN: 13

  - STATUS_NONE: 0
  - STATUS_PARALYZE: 1
  - STATUS_SLEEP: 2
  - STATUS_FREEZE: 3
  - STATUS_BURN: 4
  - STATUS_POSION: 5
  - STATUS_CONFUSE: 6
  - STATUS_INFATUATE: 7
  - STATUS_BINDING: 8
  - STATUS_NIGHTMARE: 9
  - STATUS_CURSED: 10
  - STATUS_TAUNT: 11
  - STATUS_TORMENT: 12
  - STATUS_DISABLE_LAST_MOVE: 13
  - STATUS_NEXT_TURN_SLEEP: 14
  - STATUS_HEAL_BLOCK: 15
  - STATUS_DISBALE_ABILITY: 16
  - STATUS_FORESIGHT: 17
  - STATUS_LEECH_SEED: 18
  - STATUS_BLOCK_ITEM: 19
  - STATUS_PERISH_SONG: 20
  - STATUS_INGRAIN: 21

  # did't test
  - STATUS_CANT_ESCAPE: 22
  - STATUS_ENCORE: 23

  - STATUS_SPECIAL_CODE: 0xFFFF
 ''')
  for x in range(len(MoveNames)):
    Personal.write(f'- MOVE_{MoveNames[x]}: {x}\n')

for Entry in sorted(PersonalExt.glob('*')):
  with open(f'move_txt/{Entry.stem[-3:]}.yml', 'w') as Personal:
    Personal.write(f'MOVE_{MoveNames[Count]}:\n') # Header
    # Write EntryData
    with Entry.open('rb') as PersonalRAW:
        Personal.write(f'- Type: {TypeNames[unpack("B", PersonalRAW.read(1))[0]]}\n')
        Personal.write(f'- Quality: {QualityNames[unpack("B", PersonalRAW.read(1))[0]]}\n')
        Personal.write(f'- Category: {SplitNames[unpack("B", PersonalRAW.read(1))[0]]}\n')
        Personal.write(f'- Power: {unpack("B", PersonalRAW.read(1))[0]}\n')
        acc = str(unpack("B", PersonalRAW.read(1))[0]).replace("101","MUST_HIT")
        Personal.write(f'- Accuracy: {acc}\n')
        Personal.write(f'- Base PP: {unpack("B", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Priority: {unpack("b", PersonalRAW.read(1))[0]}\n')
        HitMinMax = unpack("B", PersonalRAW.read(1))[0]
        Personal.write(f'- Hit: {HitMinMax & 0xF} | {HitMinMax >> 4}\n')
        status = unpack("<H", PersonalRAW.read(2))[0]
        if status == 0xFFFF:
          InflictStatus = "STATUS_SPECIAL_CODE"
        else:
          InflictStatus = StatusNames[status]
        Personal.write(f'- Inflict Status: {InflictStatus}\n')
        Personal.write(f'- Inflict Chance: {unpack("B", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Inflict Duration: {Duration[unpack("B", PersonalRAW.read(1))[0]]}\n')
        Personal.write(f'- Turn (min): {unpack("B", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Turn (max): {unpack("B", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Critical Hit Stage: {unpack("B", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Flinch Rate: {unpack("B", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Move Animation ID: {unpack("<H", PersonalRAW.read(2))[0]}\n')
        Personal.write(f'- Recoil: {unpack("b", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Heal: {unpack("b", PersonalRAW.read(1))[0]}\n')
        Personal.write(f'- Target: {Target[unpack("B", PersonalRAW.read(1))[0]]}\n')
        
        Personal.write(f'- Status Change Stats:\n')
        for x in range(3):
          Personal.write(f'  - {Stats[unpack("B", PersonalRAW.read(1))[0]]}\n')

        Personal.write(f'- Status Change Stages:\n')
        for x in range(3):
          Personal.write(f'  - {unpack("b", PersonalRAW.read(1))[0]}\n')

        Personal.write(f'- Status Change Chances:\n')
        for x in range(3):
          Personal.write(f'  - {unpack("B", PersonalRAW.read(1))[0]}\n')

        pad = unpack("H", PersonalRAW.read(2))[0]
        Personal.write(f'- Padding: 0x5353\n')

        flag = unpack("<L", PersonalRAW.read(4))[0] & 0xFFFFFFFF
        flagString = ""
        flag_idx = 0
        if flag > 0:
          while flag != 0:
            if flag & 1 != 0:
              if len(flagString) != 0:
                flagString += ' | '

              if flag_idx < len(FlagTable):
                flagString += FlagTable[flag_idx]
              else:
                print((flag & 1) << flag_idx)
                flagString += str((flag & 1) << flag_idx)
            flag_idx += 1
            flag >>= 1
          Personal.write(f'- Flags: {flagString}\n')
        else:
          Personal.write(f'- Flags: 0\n')
    Count += 1