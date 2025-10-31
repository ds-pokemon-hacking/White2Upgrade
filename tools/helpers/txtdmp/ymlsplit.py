import yaml
import yaml.loader

with open('gen6evos.yml', 'r') as EVO:
    EVO_DATA = yaml.load(EVO, Loader=yaml.loader.SafeLoader)
    print(EVO_DATA)
    i = 650
    for x in EVO_DATA.keys():
        with open(f'yeo/{i}.yml', 'w') as SPLT:
            SPLT.write(f'{x}:\n')
            SPLT.write(yaml.dump(EVO_DATA[x]))
        i += 1
