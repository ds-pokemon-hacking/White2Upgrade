#!/usr/bin/env python3
import yaml
import os
import sys

class IndentDumper(yaml.SafeDumper):
    # Ensure lists are indented properly
    def increase_indent(self, flow=False, indentless=False):
        return super().increase_indent(flow, False)

def fix_learnset_structure(data):
    """
    Converts top-level list of one-element dict entries into
    a proper mapping while keeping list indenting intact.
    """
    fixed = {}

    for species, entries in data.items():
        new_entries = {}
        for item in entries:
            if isinstance(item, dict):
                key, value = list(item.items())[0]
                new_entries[key] = value
        fixed[species] = new_entries

    return fixed


def process_file(path):
    with open(path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)

    fixed = fix_learnset_structure(data)

    with open(path, "w", encoding="utf-8") as f:
        yaml.dump(
            fixed,
            f,
            Dumper=IndentDumper,
            sort_keys=False,
            default_flow_style=False,
            indent=2
        )

    print(f"Fixed: {path}")


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 fix_learnsets.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]

    for filename in os.listdir(directory):
        if filename.lower().endswith((".yml", ".yaml")):
            process_file(os.path.join(directory, filename))


if __name__ == "__main__":
    main()
