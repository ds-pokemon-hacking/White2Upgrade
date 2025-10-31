from pathlib import Path
import yaml

def flatten_dict(iterable):
    flat = {}
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

def load_defines(path, defines : dict):
    with Path(path).open('r') as INCLUDE_RAW:
        INCLUDE = yaml.safe_load(INCLUDE_RAW)
        defines |= flatten_dict(INCLUDE['DEFINE'])

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
