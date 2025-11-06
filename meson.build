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

# Tools
subdir('tools')

# Custom code patches.  
subdir('src')

# Assets
subdir('data')
  
# The ROM itself
rom = custom_target('White2Upgrade.nds',
  output: 'White2Upgrade.nds',
  input: [
    vfs_files,
  ],
  command: ['java', '-cp', ctrmap_jar, 'ctrmap.cli.ROMBuilder', '--input', 'White2Upgrade.cmproj', '--output', '@OUTPUT@']
)