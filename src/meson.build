patches = []

subdir('pokedex_expansion')
subdir('type_expansion')

cxx = meson.get_compiler('cpp')
includes = [
    # Important libraries.
    '-I' + meson.project_source_root() / 'include' / 'extlib',
    '-I' + meson.project_source_root() / 'include' / 'NitroKernel',
    '-I' + meson.project_source_root() / 'include' / 'swan',
    '-I' + meson.project_source_root() / 'include',

    # Custom code patches.
    '-I' + meson.project_source_root() / 'src' / 'pokedex_expansion',
    '-I' + meson.project_source_root() / 'src' / 'type_expansion',
]

sources = [
    pokedex_expansion,
    type_expansion,
]

w2u_main_elf = custom_target(
  'w2u_main.elf',
  input: sources,
  output: 'w2u_main.elf',
  command: ['arm-none-eabi-g++', cpp_args, includes ,'-o', '@OUTPUT@', '@INPUT@'],
)

w2u_main_dll = custom_target(
  'w2u_main.dll',
  input: w2u_main_elf,
  output: 'w2u_main.dll',
  command: ['java', '-cp', rpmauthoring_jar, 'rpm.cli.RPMTool', 
        '-i', '@INPUT@', 
        '--fourcc', 'DLXF', 
        '-o', meson.project_source_root() / 'vfs' / 'data' / 'patches' / 'w2u_main.dll', 
        '--esdb', meson.project_source_root() / 'pmc' / 'IRDO.yml', 
    ],
)