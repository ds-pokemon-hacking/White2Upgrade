patches = []

subdir('pokedex_expansion')
subdir('type_expansion')
subdir('pwan_animation')

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
    '-I' + meson.project_source_root() / 'src' / 'pwan_animation',
]

sources = [
    pokedex_expansion,
    type_expansion,
    pwan_animation,
    files('../include/util/filesystem.cpp'),
    files('battle_setup_patch.s'),
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
  command: ['sh', '-c',
        'java -cp "$1" rpm.cli.RPMTool -i "$2" --fourcc DLXF -o "$3" --esdb "$4" --generate-relocations && cp "$3" "$5"',
        'rpmtool',
        ctrmap_jar,
        '@INPUT@',
        '@OUTPUT@',
        meson.project_source_root() / 'pmc' / 'IRDO.yml',
        meson.project_source_root() / 'vfs' / 'data' / 'patches' / 'w2u_main.dll',
    ],
)
