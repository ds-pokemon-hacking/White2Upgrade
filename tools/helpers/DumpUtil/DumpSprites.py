from pathlib import Path
from struct import unpack
from subprocess import run
from lzss import *
import math, shutil

WkPath = Path('sprites')
# if WkPath.exists():
#     shutil.rmtree(WkPath.as_posix())
# run(['tools/knarc/knarc', '-d', WkPath.as_posix(), '-u', '4.arc'])
for File in WkPath.glob('*'):
    if File.stat().st_size > 0:
        FileData = None
        with File.open('rb') as Hndl:
            # Check if file is LZ-compressed.
            FileData = None
            IsLZ = Hndl.read(1) 
            Hndl.seek(0)
            # LZ11; need to decompress before continuing.
            if IsLZ == b'\x11':
              try:
                FileData = decompress_file(Hndl)
                if len(FileData) == 0:
                    Hndl.seek(0)
                    FileData = Hndl.read()
              except DecompressionError as e:
                Hndl.seek(0)
                FileData = Hndl.read()
            else:
                FileData = Hndl.read()
        # Decode the file based on the extension.
        print(File.as_posix())
        match unpack('<I', FileData[0x0 : 0x4])[0]:
            case 0x4E434752:
                # NCGR.
                NewFile = None
                if IsLZ:
                    NewFile = Path(File.parent / f'{File.stem}.ncgr')
                    NewFile.write_bytes(FileData)
                    File.unlink()
                else:
                    NewFile = File.rename(File.parent / f'{File.stem}.ncgr')
            case 0x4E434C52:
                # NCLR.
                NewFile = File.rename(File.parent / f'{File.stem}.nclr')
            case 0x4E434552:
                # NCER.
                NewFile = File.rename(File.parent / f'{File.stem}.ncer')
            case 0x4E4D4352:
                # NMCR.
                NewFile = File.rename(File.parent / f'{File.stem}.nmcr')
            case 0x4E414E52:
                # NANR.
                if IsLZ:
                    NewFile = Path(File.parent / f'{File.stem}.nanr')
                    NewFile.write_bytes(FileData)
                    File.unlink()
                else:
                    NewFile = File.rename(File.parent / f'{File.stem}.nanr')
            case 0x4E4D4152:
                # NMAR.
                NewFile = File.rename(File.parent / f'{File.stem}.nmar')
            case _:
                NewFile = File.rename(File.parent / f'{File.stem}.bin')
    else:
        NewFile = File.rename(File.parent / f'{File.stem}.bin')
            