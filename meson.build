project('White2Upgrade', 'cpp', 
    version: '0.0.1')

fs = import('fs')
incdir = include_directories('include')

cpp_args = [
    '-mthumb',
    '-march=armv5t',
    '-r',
    '-nostdinc++'
]

asm_args = [
    '-mthumb',
    '-march=armv5t',
    '-r'
]

python = find_program('python3', 'python', required: true)

# Custom code patches.  
subdir('src')

# Assets
subdir('user')

# The ROM itself
rom = custom_target('White2Upgrade.nds',
  output: 'White2Upgrade.nds',
  input: [
    vfs_files
  ],
  command: ['java', '-cp', 'tools/CTRMap/CTRMapV-dirty.jar', 'ctrmap.cli.ROMBuilder', '--input', 'White2Upgrade.cmproj', '--output', '@OUTPUT@']
)