from pathlib import Path
from argparse import ArgumentParser

# Import serializer base class.
from impl.serializer import *

# Import individual serializers.
from impl.generic import *
from impl.enc import *
from impl.lz import *
from impl.msg import *
from impl.trainer import *
from impl.y9 import *

def main():
    Parser = ArgumentParser()
    Parser.add_argument('type', metavar='t', type=str)
    Parser.add_argument('input', metavar='i', type=str)
    Parser.add_argument('output', metavar='o', type=str)
    Parser.add_argument('ex_parameters', nargs='*')
    Arguments = Parser.parse_args()
    SerializerObject = None
    print(Arguments.type)
    match Arguments.type:
        case 'generic':
            SerializerObject = GenericSerializer()
        case 'encounter':
            SerializerObject = EncounterSerializer()
        case 'lz':
            SerializerObject = LZSerializer()
        case 'text':
            SerializerObject = MessageSerializer()
        case 'trainer':
            SerializerObject = TrainerSerializer()
        case 'y9':
            SerializerObject = OverlayTableSerializer()
    Arguments.ex_parameters = format_extra_parameters(Arguments.ex_parameters)
    SerializerObject.Serialize(Arguments)
        
if __name__ == '__main__':
    exit(main())