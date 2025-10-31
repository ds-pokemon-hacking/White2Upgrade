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
# 
Count = 0
for Entry in sorted(PersonalExt.glob('*')):
    with open(f'items_txt/{Entry.stem[-3:]}.yml', 'w') as PERSONAL_ENTRY:      
      PERSONAL_ENTRY.write(f'ITEM_{Names[Count]}:\n') # Header
      with Entry.open('rb') as PersonalRAW:
        # Write EntryData
        with Entry.open('rb') as PersonalRAW:
            PERSONAL_ENTRY.write(f'  - Price: {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Held Effect: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Held Argument: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Natural Gift Effect: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Fling Effect: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Fling Power: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Natural Gift Power: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Packed: {unpack("<H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Effect Field: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Effect Battle: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Has Battle Stats: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Item Class: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Consumable: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Sort Index: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'  - Battle Stats:\n')
            PERSONAL_ENTRY.write(f'    - Cure Inflict: {unpack("B", PersonalRAW.read(1))[0]}\n')
          
            PERSONAL_ENTRY.write(f'    - Boost:\n')
            for x in range(4):
              PERSONAL_ENTRY.write(f'      - {unpack("B", PersonalRAW.read(1))[0]}\n')
            
            PERSONAL_ENTRY.write(f'    - Function Flags: {unpack("H", PersonalRAW.read(2))[0]}\n')
            PERSONAL_ENTRY.write(f'    - EV (HP): {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - EV (ATK): {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - EV (DEF): {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - EV (SPE): {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - EV (SPA): {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - EV (SPD): {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - Heal Amount: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - PP Gain: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - Friendship 1: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - Friendship 2: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - Friendship 3: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - field_1F: {unpack("B", PersonalRAW.read(1))[0]}\n')
            PERSONAL_ENTRY.write(f'    - field_20: {unpack("B", PersonalRAW.read(1))[0]}\n')
        Count += 1
