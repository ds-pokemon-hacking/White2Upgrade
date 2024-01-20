from pathlib import Path
from struct import unpack

SpeciesNames = []
EncountersBinary = Path('encounters')
EncountersTXT = Path('../../../data/encounters')

with open('../txtdmp/Species.txt') as SpeciesFile:
    while (CurrSpecies := SpeciesFile.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])
    pass

# FileBeginning = ["    - GRASS_SINGLES", "    - GRASS_DOUBLES", "    - GRASS_RARE", "    - SURF_SINGLES", "    - SURF_RARE", "    - FISH_SINGLES", "    - FISH_RARE", "    - UNKNOWN"]
# Seasons = ["SPRING", "SUMMER", "FALL", "WINTER"]
# GrassSlots = ["        - 20%", "        - 20%", "        - 10%", "        - 10%", "        - 10%", "        - 10%", "        - 5%", "        - 5%", "        - 4%", "        - 4%", "        - 1%", "        - 1%"]
# WaterSlots = ["        - 60%", "        - 30%", "        - 5%", "        - 4%", "        - 1%"]

# def GetFormIndex(Season, ByteForm):
#     #todo
#     return 0

# Get all of the files in the NARC folder then convert to YAML
for x in EncountersBinary.glob('*'):
    # Open the source file for reading (binary mode)
    with x.open('rb') as ENC_RAW:
        # Open txt file for writing (txt mode)
        with (EncountersTXT / f'{x.stem}.yml').open('w') as ENC_TXT:
            # Determine number of sections from ENC_RAW size.
            # Each container is expected to be 0xE8, so it would be size / 0xE8 (// does integer division)
            section_count = x.stat().st_size // 0xE8
            for k in range(section_count):
                ENC_TXT.write(f'ENCOUNTER_CONTAINER_{k}:\n')
                ENC_TXT.write(f'  GRASS_SINGLES: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  GRASS_DOUBLES: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  GRASS_RARE: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  SURF_SINGLES: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  SURF_RARE: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  FISH_SINGLES: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  FISH_RARE: {unpack("B", ENC_RAW.read(1))[0]}\n')
                ENC_TXT.write(f'  UNKNOWN: {unpack("B", ENC_RAW.read(1))[0]}\n')
                for Category in ['GRASS', 'SURF', 'FISH']:
                    ENC_TXT.write(f'  {Category}:\n')
                    for EncType in ['SINGLES', 'DOUBLES', 'SPECIAL']:
                        if EncType == 'DOUBLES' and Category != 'GRASS':
                            continue
                        ENC_TXT.write(f'    {EncType}:\n')
                        match Category:
                            case 'GRASS':
                                upper_limit = 0xC
                            case _:
                                upper_limit = 0x5
                        for x in range(upper_limit):
                            ENC_TXT.write(f'      Slot {x}:\n')
                            #  Parse Species and Form
                            species = unpack("<H", ENC_RAW.read(2))[0]
                            form = (species >> 11) & 0x1F
                            species &= 0x7FF
                            
                            ENC_TXT.write(f'        Species: SPECIES_{SpeciesNames[species] if species < len(SpeciesNames) else species}\n')
                            ENC_TXT.write(f'        Form: {form}\n')
                            ENC_TXT.write(f'        Minimum Level: {unpack("B", ENC_RAW.read(1))[0]}\n')
                            ENC_TXT.write(f'        Maximum Level: {unpack("B", ENC_RAW.read(1))[0]}\n')