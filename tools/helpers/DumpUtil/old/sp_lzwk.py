from pathlib import Path
from lzss import decompress_file

WkPath = Path('sprites')

for File in WkPath.glob('*.lz11'):
    DecmpBuff = b''
    with File.open('rb') as Hndl:
        DecmpBuff = decompress_file(Hndl)
    Extension = DecmpBuff[:4][::-1].decode('utf8')
    File.unlink()
    with Path(f'sprites/{File.stem}.{Extension.lower()}').open('wb') as Hndl:
        Hndl.write(DecmpBuff)