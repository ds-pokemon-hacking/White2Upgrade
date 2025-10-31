subdir('pokedex_expansion')
subdir('type_expansion')

main = executable('w2u_main',
    sources: [
        pokedex_expansion,
        type_expansion,
    ],
    include_directories: include_directories(
        # Important libraries.
        '../include/extlib',
        '../include/NitroKernel',
        '../include/swan',
        '../include',

        # Custom code patches.
        'pokedex_expansion',
        'type_expansion',
    ),
    name_suffix: 'elf',
    cpp_args: cpp_args,
    link_args: link_args,
)