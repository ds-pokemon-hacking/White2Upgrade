project('White2Upgrade', 'cpp', version: '0.0.1')

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

pmc_arm9 = custom_target(
  'pmc_arm9',
  input: meson.project_source_root() / 'pmc' / 'ARM9PMC.bin',
  output: 'pmc_arm9.stamp',
  command: ['sh', '-c',
    'cp "$1" "$2" && touch "$3"',
    'pmc_arm9',
    '@INPUT@',
    meson.project_source_root() / 'vfs' / 'arm9.bin',
    '@OUTPUT@'
  ],
)

pmc_overlay = custom_target(
  'pmc_overlay',
  input: meson.project_source_root() / 'pmc' / 'OVL344.bin',
  output: 'pmc_overlay.stamp',
  command: ['sh', '-c',
    'mkdir -p "$(dirname "$2")" && cp "$1" "$2" && touch "$3"',
    'pmc_overlay',
    '@INPUT@',
    meson.project_source_root() / 'vfs' / 'overlay' / 'overlay_0344.bin',
    '@OUTPUT@'
  ],
)
  
# The ROM itself
rom = custom_target('White2Upgrade.nds',
  output: 'White2Upgrade.nds',
  input: [
    vfs_files,
    w2u_main_dll,
    pmc_arm9,
    pmc_overlay
  ],
  command: ['java', '-cp', ctrmap_jar, 'ctrmap.cli.ROMBuilder', '--input', meson.project_source_root() / 'White2Upgrade.cmproj', '--output', '@OUTPUT@']
)
