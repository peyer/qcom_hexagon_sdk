import argparse
from YAMLMapParser import YAMLFile


def main():
    parser = argparse.ArgumentParser(description='YAML parser', usage='%(prog)s YAML_file', formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('yaml_file', help='The input YAML file.')
    args = parser.parse_args()
    with open(args.yaml_file, 'r') as f:
        yamlfile = YAMLFile(f)

    """ Get all of the output sections """
    osections = yamlfile.get_output_sections()
    """ Iterate through the output sections """
    for osection in osections:
        print(osection['Name'])
        """ Get all of the input sections """
        isections = osection['Contents']
        if isections is not None:
            """ Iterate through all of the input sections """
            for isection in isections:
                if isection is not None and isection['Name'] != "ALIGNMENT_PADDING":
                    print('\t' + isection['Name'] + '\t' + isection['Origin'])
                    """ Get all of the symbols for this input section """
                    symbols = isection['Symbols']
                    for symbol in symbols:
                        print('\t\t' +  hex(symbol['Value']) + '\t' + symbol['Symbol'])

if __name__ == '__main__':
    main()
