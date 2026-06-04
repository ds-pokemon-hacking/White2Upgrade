pokegra_battle_arc = 'a' / '0' / '0' / '4'
pokegra_icons_arc = 'a' / '0' / '0' / '7'
pokegra_footprints_arc = 'a' / '1' / '6' / '5'
pokegra_footprints_count = 832
arc_marker_text = '.arc\ncompress default auto\n'
build_pokegra_battle_py = find_program(meson.project_source_root() / 'tools' / 'graphics' / 'build_pokegra_battle.py', native: true)

vfs_files += custom_target(
  'stage_pokegra_battle',
  output: 'stage_pokegra_battle.stamp',
  command: [
    build_pokegra_battle_py,
    '--source', meson.current_source_dir() / 'pokegra',
    '--battle-vfs', meson.project_source_root() / 'vfs' / 'data' / pokegra_battle_arc,
    '--pwan-vfs', meson.project_source_root() / 'vfs' / 'data' / 'pokeweb_pwan',
    '--arc-text', arc_marker_text,
    '--stamp', '@OUTPUT@',
  ],
  build_always_stale: true,
)

vfs_files += custom_target(
  'stage_pokegra_icons',
  output: 'stage_pokegra_icons.stamp',
  command: ['sh', '-eu', '-c', '''
    src="$1"
    dst="$2"
    stamp="$3"
    rm -rf "$dst"
    mkdir -p "$dst"
    for file in "$src"/*; do
      base="${file##*/}"
      case "$base" in
        007_[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].*)
          entry="${base#007_}"
          entry="${entry%.*}"
          cp "$file" "$dst/$((10#$entry))"
          ;;
        07_[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].*)
          entry="${base#07_}"
          entry="${entry%.*}"
          cp "$file" "$dst/$((10#$entry))"
          ;;
        *)
          cp "$file" "$dst/$base"
          ;;
      esac
    done
    printf "%s" "$4" > "$dst/.arc"
    touch "$stamp"
  ''',
    'stage_pokegra_icons',
    meson.current_source_dir() / 'pokegra' / 'icons',
    meson.project_source_root() / 'vfs' / 'data' / pokegra_icons_arc,
    '@OUTPUT@',
    arc_marker_text,
  ],
  build_always_stale: true,
)

vfs_files += custom_target(
  'stage_pokegra_footprints',
  output: 'stage_pokegra_footprints.stamp',
  command: ['sh', '-eu', '-c', '''
    src="$1"
    dst="$2"
    stamp="$3"
    count="$5"
    rm -rf "$dst"
    mkdir -p "$dst"
    i=0
    while [ "$i" -lt "$count" ]; do
      file=$(printf "$src/165_%08d.bin" "$i")
      cp "$file" "$dst/$i"
      i=$((i + 1))
    done
    printf "%s" "$4" > "$dst/.arc"
    touch "$stamp"
  ''',
    'stage_pokegra_footprints',
    meson.current_source_dir() / 'pokegra' / 'footprints',
    meson.project_source_root() / 'vfs' / 'data' / pokegra_footprints_arc,
    '@OUTPUT@',
    arc_marker_text,
    pokegra_footprints_count.to_string(),
  ],
  build_always_stale: true,
)
