from pathlib import Path

WkPath = Path('sprites')

for File in WkPath.glob('*'):
    if File.stat().st_size > 0:
        with File.open('rb') as Hndl:
            Extension = None
            match Hndl.read(1)[0]:
                case 0x11:
                    Extension = 'lz11'
                case _:
                    Hndl.seek(0, 0)
                    Magic = ""
                    for x in range(4):
                        Magic += chr(Hndl.read(1)[0]) 
                    Magic = Magic[::-1].lower()
                    Extension = Magic if str.isalpha(Magic) else 'bin'
            Hndl.close()
            File.rename(f'sprites/{File.stem}.{Extension}')
    else:
        File.unlink()