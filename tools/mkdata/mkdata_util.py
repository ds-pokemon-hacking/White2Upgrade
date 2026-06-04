from pathlib import Path
import yaml
import tomllib

def flatten(iterable):
    flat = {}
    if isinstance(iterable, dict):
        for e in iterable.items():
            if isinstance(e, tuple):
                e_ = { e[0] : e[1] }
            else:
                e_ = e
            flat |= e_
    else:
        for e in iterable:
            flat |= e
    return flat

def flatten_yaml_tree(items):
    data = []
    if len(items) == 0:
        return data
    for item in items:
        flatten_yaml_tree_helper(item, data)
    return data

def flatten_yaml_tree_helper(item, data):
    # Check if we are working with a scalar type (string/number in this case).
    seq_type = type(item)
    if seq_type == list:
        # Call unroll sequence again on "item".
        data += flatten_yaml_tree(item)
    elif seq_type == dict:
        # Treat as a sequence. 
        data += flatten_yaml_tree(list(item.values()))
    else:
        data.append(item)

def resolve_label(item, defines):
    if type(item) == str:
        # Will err if it doesn't exist.
        return defines[item]
    return item

def resolve_metadata_path(path):
    source = Path(path)
    if source.suffix in {'.yml', '.yaml'}:
        toml_source = source.with_suffix('.toml')
        if toml_source.exists():
            return toml_source
    return source

def load_metadata(path):
    source = resolve_metadata_path(path)
    if source.suffix == '.toml':
        with source.open('rb') as source_raw:
            return tomllib.load(source_raw)
    with source.open('r') as source_raw:
        return yaml.safe_load(source_raw)

def load_defines(path, defines : dict):
    INCLUDE = load_metadata(path)
    defines |= flatten(INCLUDE['DEFINE'])

def load_source_data(path):
    source = Path(path)
    if source.suffix == '.toml':
        with source.open('rb') as source_raw:
            return tomllib.load(source_raw)
    with source.open('r') as source_raw:
        return yaml.safe_load(source_raw)

def format_extra_parameters(params: list):
    parameters = {}
    if len(params) == 0:
        return parameters
    
    if len(params) % 2 != 0:
        print('Uneven extra parameter count; might be a mismatch. Exiting')
        exit(1)
    
    for parameter_index in range(0, len(params), 2):
        parameters[params[parameter_index]] = params[parameter_index + 1]

    return parameters
