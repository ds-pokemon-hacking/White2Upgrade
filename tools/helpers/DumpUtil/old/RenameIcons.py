from pathlib import Path
from subprocess import run
import shutil

WkPath = Path('src/arc/pokegra_iconsa/icons')

for File in WkPath.glob('*'):
    New = File.rename(File.parent / f'007_{str(int(File.stem[2:])).zfill(8)}.png')
    print(New.as_posix())