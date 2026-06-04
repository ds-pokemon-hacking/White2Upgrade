game_text = run_command(
    'sh',
    '-c',
    'find "$1" -maxdepth 1 -type f -name "*.toml" -printf "%f\n" | sed "s/\\.toml$//" | sort -n',
    'sh',
    meson.current_source_dir() / 'game',
    check: true,
).stdout().strip().split('\n')

system_text = run_command(
    'sh',
    '-c',
    'find "$1" -maxdepth 1 -type f -name "*.toml" -printf "%f\n" | sed "s/\\.toml$//" | sort -n',
    'sh',
    meson.current_source_dir() / 'system',
    check: true,
).stdout().strip().split('\n')

text = [
    game_text,
    system_text
]
