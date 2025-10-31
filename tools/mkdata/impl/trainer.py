from pathlib import Path
from argparse import ArgumentParser
import yaml
import struct
from .serializer import Serializer
from mkdata_util import *

class TrainerSerializer(Serializer):
    def __init__(self):
        super().__init__()

    def Serialize(self, Arguments: dict) -> None:
        Input, Output = Path(Arguments.input), Path(Arguments.output)
        defines = {}
        if not Input.exists():
            print(f'"{Arguments.input}" does not exist. Exiting.')
            return 1

        with Input.open('r') as DATA:
            TrainerConfiguration = yaml.safe_load(DATA)

            # Traverse data and resolve to integers
            UserSpecifiedMoves = any(['MOVES' in x for x in TrainerConfiguration['PARTY']])
            UserSpecifiedHeldItem = any(['HELD_ITEM' in x for x in TrainerConfiguration['PARTY']])

            [load_defines(x, defines) for x in [
                'tools/mkdata/enum/species.yml',
                'tools/mkdata/enum/moves.yml',
                'tools/mkdata/enum/items.yml',
                'tools/mkdata/enum/btl_gender.yml',
                'tools/mkdata/enum/btl_abil.yml',
            ]]

            if Arguments.ex_parameters['output_type'] == 'trpoke':
                with Output.open('wb') as TRPOKE:
                    for member in TrainerConfiguration['PARTY']:
                        TRPOKE.write(struct.pack('B', resolve_label(member['DIFFICULTY_VALUE'], defines)))
                        TRPOKE.write(struct.pack('B', (resolve_label(member['ABILITY'], defines) << 0x4) | resolve_label(member['GENDER'], defines) & 0xF))
                        TRPOKE.write(struct.pack('<H', resolve_label(member['LEVEL'], defines)))
                        TRPOKE.write(struct.pack('<H', resolve_label(member['SPECIES'], defines)))
                        TRPOKE.write(struct.pack('<H', resolve_label(member['FORM'], defines)))
                        if UserSpecifiedHeldItem:
                            TRPOKE.write(struct.pack('<H', resolve_label(member['HELD_ITEM'], defines)))
                        if UserSpecifiedMoves:
                            for move in member['MOVES']:
                                TRPOKE.write(struct.pack('<H', resolve_label(move, defines)))
            elif Arguments.ex_parameters['output_type'] == 'trdata':
                with Output.open('wb') as TRDATA:
                    # Determine format.
                    TRPOKE_FORMAT = (1 << 1 if UserSpecifiedHeldItem else 0) | (1 if UserSpecifiedMoves else 0)
                    TRDATA.write(struct.pack('B', TRPOKE_FORMAT))
                    TRDATA.write(struct.pack('B', resolve_label(TrainerConfiguration['CLASS'], defines)))
                    TRDATA.write(struct.pack('B', resolve_label(TrainerConfiguration['BATTLE_TYPE'], defines)))
                    TRDATA.write(struct.pack('B', len(TrainerConfiguration['PARTY'])))
                    for item in TrainerConfiguration['ITEMS']:
                        TRDATA.write(struct.pack('<H', resolve_label(item, defines)))
                    TRDATA.write(struct.pack('<I', TrainerConfiguration['AI']))
                    TRDATA.write(struct.pack('B', TrainerConfiguration['CAN_HEAL']))
                    TRDATA.write(struct.pack('B', TrainerConfiguration['REWARD_MONEY']))
                    TRDATA.write(struct.pack('<H', resolve_label(TrainerConfiguration['REWARD_ITEM'], defines)))