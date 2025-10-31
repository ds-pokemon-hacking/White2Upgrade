from argparse import ArgumentParser
from struct import pack
import yaml
from yaml.loader import SafeLoader
from .serializer import Serializer

class OverlayTableSerializer(Serializer):
    def __init__(self):
        super().__init__()

    def Serialize(self, Arguments: dict) -> None:
        OVT_DATA = None
        with open(Arguments.input, 'r') as OVT_TXT:
            OVT_DATA = yaml.load(OVT_TXT, Loader=SafeLoader)
        print(Arguments)
        with open(Arguments.output, 'wb') as OVT:
            for OVL in OVT_DATA:
                ENTRY_DATA = OVT_DATA[OVL]
                OVT.write(pack('<I', OVL))
                OVT.write(pack('<I', ENTRY_DATA['MountAddress']))
                OVT.write(pack('<I', ENTRY_DATA['MountSize']))
                OVT.write(pack('<I', ENTRY_DATA['BSSSize']))
                OVT.write(pack('<I', ENTRY_DATA['StaticInitializerAddress']))
                OVT.write(pack('<I', ENTRY_DATA['StaticInitializerEndAddress']))
                OVT.write(pack('<I', ENTRY_DATA['FileID']))
                # It's all decmped anyway.
                OVT.write(pack('<I', ENTRY_DATA['MountSize']))