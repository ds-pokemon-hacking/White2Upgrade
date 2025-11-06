vfs_files = []
vfs_build_dir = meson.project_build_root() / 'vfs' / 'data'
relative_source_root = fs.relative_to(meson.project_source_root(), meson.project_build_root())

# Load all of the resources.
subdir('encounters')
subdir('graphics')
subdir('items')
subdir('pml/evolutions')
subdir('pml/learnsets')
subdir('pml/moves')
subdir('pml/personal')
subdir('trainers')
subdir('text')

# System Text (/a/0/0/2)
foreach system_text_file : system_text
  system_text_file_name = 'system_text_' + fs.replace_suffix(system_text_file, '')
endforeach

# Event Text (/a/0/0/3)
foreach event_text_file : event_text
  event_text_file_name = 'event_text_' + fs.replace_suffix(event_text_file, '')
endforeach

# pokegra_battle (/a/0/0/4)
# I don't know how I'm going to deal with this one...





