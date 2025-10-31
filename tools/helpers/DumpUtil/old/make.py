from pathlib import Path
from shutil import copyfile
from subprocess import run

with Path('vfs/graphics/icons') as p:
    for x in sorted(p.glob('*'), key=lambda x: int(x.stem)):
        p_c = Path(f'build/IRDO/narcs/a/0/0/7/7_{str(int(x.stem)).zfill(8)}.NCGR')
        run(['tools/nitrogfx/nitrogfx', x.as_posix(), p_c.as_posix()])
        p_c.rename(f'build/IRDO/narcs/a/0/0/7/7_{str(int(x.stem)).zfill(8)}.bin')