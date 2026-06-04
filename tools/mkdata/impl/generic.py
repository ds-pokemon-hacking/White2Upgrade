from pathlib import Path
from .serializer import Serializer
from mkdata_util import *
import struct

class GenericSerializer(Serializer):
    def __init__(self):
        super().__init__()
        
    def Serialize(self, Arguments : dict) -> None:
        # Structure file; check if it exists.
        ScriptPath = Path(__file__).resolve().parent.parent
        StructConfig = resolve_metadata_path(ScriptPath / Path(f'defs/{Arguments.ex_parameters["format"]}.toml'))
        Input, Output = Path(Arguments.input), Path(Arguments.output)
        Output.parent.mkdir(exist_ok=True, parents=True)
        if not StructConfig.exists():
            print(f'Structure configuration "{StructConfig}" does not exist. Exiting.')
            return 1
        
        # Read structure configuration.
        Configuration = load_metadata(StructConfig)
        if not 0 < len(Configuration.keys()) <= 2:
            print(f'Invalid number of keys in "{StructConfig}"!')
            return 1
        
        structure = flatten(Configuration['STRUCTURE'])
        
        defines = {}
        if 'INCLUDE' in Configuration.keys():
            for include in Configuration['INCLUDE']:
                load_defines((ScriptPath / include).as_posix(), defines)

        format_string = ''
        tree = structure.values()
        tree_flat = flatten_yaml_tree(tree)

        def add_field(size):
            nonlocal format_string
            match size:
                case 's8':
                    format_string += 'b'
                case 'u8':
                    format_string += 'B'
                case 's16':
                    format_string += 'h'
                case 'u16':
                    format_string += 'H'
                case 's32':
                    format_string += 'i'
                case 'u32':
                    format_string += 'I'
                case _:
                    format_string += 'X'

        for e in tree_flat:
            add_field(e)

        with Output.open('wb') as OUT_DATA:
            IN_DATA_RAW = load_source_data(Input)
            IN_DATA_RAW_KEYS = list(IN_DATA_RAW.keys())

            if not 0 < len(IN_DATA_RAW_KEYS) <= 1:
                print(f'Invalid number of keys in "{Arguments.input}"!')
                return 1

            def resolve(value):
                if type(value) == int:
                    return value
                if type(value) == str:
                    final_value = 0
                    tokens = [e.strip() for e in value.split('|')]
                    for token in tokens:
                        if token.isdigit():
                            final_value |= int(token)
                            continue
                        if token in defines.keys():
                            final_value |= defines[token]
                        else:
                            print(f'Label "{token}" not defined!')
                    return final_value
                return -1

            
            IN_DATA_FLAT = flatten_yaml_tree(flatten(IN_DATA_RAW[IN_DATA_RAW_KEYS[0]]).values())
            OUT_DATA_BUFFER = [resolve(item) for item in IN_DATA_FLAT]
            while len(format_string) != len(OUT_DATA_BUFFER):
                format_string = format_string[:-1]

            OUT_DATA.write(struct.pack('<' + format_string, *OUT_DATA_BUFFER))
