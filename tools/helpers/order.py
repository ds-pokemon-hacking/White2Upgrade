from pathlib import Path

SpeciesNames = []

with open('txtdmp/Species.txt') as Species:
    while (CurrSpecies := Species.readline()) != '':
        SpeciesNames.append(CurrSpecies.upper().replace('É', 'E').replace('.', '').replace('-', '').replace(' ', '_').replace('\'', '')[:-1])

wk = Path('data/personal')

with open('.knarcorder', 'w') as ORDER:
    for k in sorted(wk.glob('*.yml'), key=lambda x: int(x.stem)):
        if (idx := int(k.stem)) < len(SpeciesNames):
            ORDER.write(f'{SpeciesNames[idx].lower()}.bin\n')
            k.rename(f'{k.parent}/{SpeciesNames[idx].lower()}.yml')
        else:
            ORDER.write(f'{idx}.bin')