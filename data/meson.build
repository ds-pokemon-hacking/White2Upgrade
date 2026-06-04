vfs_files = []
vfs_build_dir = meson.project_build_root() / 'vfs' / 'data'
relative_source_root = fs.relative_to(meson.project_source_root(), meson.project_build_root())
gen5_text_py = find_program(meson.project_source_root() / 'tools' / 'text' / 'gen5_text.py', native: true)

# Load all of the resources.
subdir('encounters')
subdir('graphics')
subdir('items')
subdir('pml')
subdir('trainers')
subdir('text')

# System Text (/a/0/0/2)
system_text_arc = 'a' / '0' / '0' / '2'
system_text_files = []
foreach system_text_file : system_text
  system_text_files += meson.current_source_dir() / 'text' / 'system' / (system_text_file + '.toml')
endforeach

vfs_files += custom_target(
  'build_system_text_narc',
  input: system_text_files,
  output: 'build_system_text_narc.stamp',
  command: [gen5_text_py, 'build-dir', meson.current_source_dir() / 'text' / 'system', meson.project_source_root() / 'vfs' / 'data' / system_text_arc, '--stamp', '@OUTPUT@'],
)

# Game/Map Text (/a/0/0/3)
game_text_arc = 'a' / '0' / '0' / '3'
game_text_files = []
foreach game_text_file : game_text
  game_text_files += meson.current_source_dir() / 'text' / 'game' / (game_text_file + '.toml')
endforeach

vfs_files += custom_target(
  'build_game_text_narc',
  input: game_text_files,
  output: 'build_game_text_narc.stamp',
  command: [gen5_text_py, 'build-dir', meson.current_source_dir() / 'text' / 'game', meson.project_source_root() / 'vfs' / 'data' / game_text_arc, '--stamp', '@OUTPUT@'],
)

# pokegra/battle (/a/0/0/4)
# I don't know how I'm going to deal with this one...
