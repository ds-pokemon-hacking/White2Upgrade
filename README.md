# Pokémon White 2 Upgrade
This repository aims to bring new features to the Generation V Pokémon game, Pokémon White 2. This should also work for Black 2, given the conditions are satisfied (proper symbols database, PMC port).

## Current Features
- Expanded Pokédex (currently up to 721).
- Fairy type.

## Setup
You will need the following tools to use this repository:
- [arm-none-eabi-{as, gcc, ld}](https://developer.arm.com/downloads/-/gnu-rm).
- [CTRMap (Community Edition)](https://github.com/kingdom-of-ds-hacking/CTRMap-CE/releases).
- gcc
- GNU Make (anything over 3.81).
- Java 1.8
- Python 3.4+
- [ndstool (2.2.0)](https://github.com/devkitPro/ndstool)
- A Linux environment (native or Windows Subsystem for Linux)

## Installation
### Initial Setup
1) Clone this repository using `git clone --recursive git@github.com:PlatinumMaster/White2Upgrade.git`, then `cd White2Upgrade`. **Do not omit the recursive flag, or you will not have all of the submodules associated with this repository.**
2) Download [CTRMap (Community Edition)](https://github.com/kingdom-of-ds-hacking/CTRMap-CE/releases), and put `CTRMap.jar` in `tools/CTRMap`, making the directory if it does not exist.
3) Grab a fresh American Pokémon White 2 ROM (preferrably one with SHA256SUM `3e50aec3db401332175a5d2b5fe2a68ac1a05ec63995dba9d1506b1b51837446`), name it `IRDO.nds`, and place it in the root directory of the repository (i.e in the same folder as `Makefile`).
4) Run `make tools` to build all of the tools associated.
5) Run `make base` to prepare the ROM contents for the build.

### Building
Once the repository is setup, run `make -j$(nproc)`. If all goes well, you shall see `White2Upgrade.nds` at the end of your build.

## Creators
- [PlatinumMaster](https://github.com/PlatinumMaster)
- [Dararo](https://github.com/Paideieitor)
- [BluRose](https://github.com/BluRosie)
- [SpagoAsparago](https://github.com/SpagoAsparago)
- [Bubble791](https://github.com/Bubble791)
- [BromBromBromley](https://github.com/BromBromBromley)
- [totally_anonymous](https://github.com/totallyanon)

## Credits
- [Bond697](https://github.com/Bond697) - Initial Generation V research.
- [KazoWAR](https://projectpokemon.org/home/forums/topic/33493-project-721/) - Project 721 (which this was based on).
- [MeroMero](https://projectpokemon.org/home/profile/50874-meromero/?tab=activity#) - Initial fairy type implementation.
- [Hello007](https://github.com/HelloOO7) - CTRMap, PMC, code injection tools, Generation V research.
- [BluRose](https://github.com/BluRosie) - Fairy type fixes, expansion fixes.
- [PlatinumMaster](https://github.com/PlatinumMaster) - Expansion fixes, PMC, code injection tools, Generation V research, build system.

