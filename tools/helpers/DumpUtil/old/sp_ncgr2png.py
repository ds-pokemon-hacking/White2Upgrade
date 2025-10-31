from subprocess import run
from pathlib import Path
import time

# Gonna do some advanced mathematics here. 
# 0, 2, 9, 11 = ncgr
# 18: nclr (Normal)
# 19: nclr (Shiny)
Cap = 17940
Base = 0
Basepath = Path('sprites')

while Base < Cap:
    # Dump data with non-shiny palettes.
    Front_Cmp = Basepath / f'4_{str(Base + 0).zfill(8)}.ncgr'
    Front_Decmp = Basepath / f'4_{str(Base + 2).zfill(8)}.ncgr'
    Back_Cmp = Basepath / f'4_{str(Base + 9).zfill(8)}.ncgr'
    Back_Decmp = Basepath / f'4_{str(Base + 11).zfill(8)}.ncgr'
    Pal_Norm = Basepath / f'4_{str(Base + 18).zfill(8)}.nclr'
    Pal_Shiny = Basepath / f'4_{str(Base + 19).zfill(8)}.nclr'
    
    print(Base)
    # Front sprites
    run(['../../tools/nitrogfx/nitrogfx', Front_Cmp.as_posix(), (Basepath / f'{Front_Cmp.stem}.png').as_posix(), '-palette', Pal_Norm.as_posix()])
    run(['../../tools/nitrogfx/nitrogfx', Front_Decmp.as_posix(), (Basepath / f'{Front_Decmp.stem}.png').as_posix(), '-palette', Pal_Norm.as_posix()])

    # Back sprites
    run(['../../tools/nitrogfx/nitrogfx', Back_Cmp.as_posix(), (Basepath / f'{Back_Cmp.stem}.png').as_posix(), '-palette', Pal_Norm.as_posix()])
    run(['../../tools/nitrogfx/nitrogfx', Back_Decmp.as_posix(), (Basepath / f'{Back_Decmp.stem}.png').as_posix(), '-palette', Pal_Norm.as_posix()])
    
    # Palettes
    run(['../../tools/nitrogfx/nitrogfx', Pal_Norm.as_posix(), (Basepath / f'{Pal_Norm.stem}.pal').as_posix(), '-bitdepth', '4'])
    run(['../../tools/nitrogfx/nitrogfx', Pal_Shiny.as_posix(), (Basepath / f'{Pal_Shiny.stem}.pal').as_posix(), '-bitdepth', '4'])
    Base += 20