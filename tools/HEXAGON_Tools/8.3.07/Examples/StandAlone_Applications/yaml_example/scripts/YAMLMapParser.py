import argparse
import os
import sys
import yaml
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

class YAMLFile(object):
    def __init__(self, yamlfile):
        self.yamlfile = yaml.load(yamlfile, Loader=Loader)
        self.grand_totals = {}
        self.objs = {}
        self.obj_totals = {}
        self.lib_objs = {}
        self.lib_totals = {}
        self.libs = {}
        self.generated = {}
        self.unused = {}
        self.object_map_code = {}
        self.unused_symbols = []

    def get_architecture(self):
        '''Return Header->Architecture.'''
        return self.yamlfile['Header']['Architecture']

    def get_emulation(self):
        '''Return Header->Emulation.'''
        return self.yamlfile['Header']['Emulation']

    def get_address_size(self):
        '''Return Header->AddressSize.'''
        return self.yamlfile['Header']['AddressSize']

    def get_output_sections(self):
        return self.yamlfile['OutputSections']

    def is_code(self, item):
        '''Return True if item is code, otherwise return False.'''
        res = False
        if 'Type' in item and 'Permissions' in item:
            permission = item['Permissions']
            if ('SHT_PROGBITS' in item['Type'] and
                'SHF_ALLOC' in permission and
                'SHF_EXECINSTR' in permission and
                not 'SHT_REL' in item['Type']):
                res = True
        return res

    def is_ro_data(self, item):
        '''Return True if item is RO data, otherwise return False.'''
        res = False
        if 'Type' in item and 'Permissions' in item:
            permission = item['Permissions']
            if ('SHT_PROGBITS' in item['Type'] and
                'SHF_ALLOC' in permission and
                not 'SHF_WRITE' in permission and
                not 'SHF_EXECINSTR' in permission and
                not 'SHT_REL' in item['Type']):
                res = True
        return res

    def is_rw_data(self, item):
        '''Return True if item is RW data, otherwise return False.'''
        res = False
        if 'Type' in item and 'Permissions' in item:
            permission = item['Permissions']
            if ('SHT_PROGBITS' in item['Type'] and
                'SHF_ALLOC' in permission and
                'SHF_WRITE' in permission and
                not 'SHF_EXECINSTR' in permission and
                not 'SHT_REL' in item['Type']):
                res = True
        return res

    def is_zi_data(self, item):
        '''Return True if item is ZI data, otherwise return False.'''
        res = False
        if 'Type' in item and 'Permissions' in item:
            permission = item['Permissions']
            if ('SHT_NOBITS' in item['Type'] and
                'SHF_ALLOC' in permission and
                'SHF_WRITE' in permission and
                not 'SHF_EXECINSTR' in permission and
                not 'SHT_REL' in item['Type']):
                res = True
        return res

    def is_debug(self, item):
        '''Return True if item is debug, otherwise return False.'''
        res = False
        if 'Type' in item and 'Permissions' in item:
            permission = item['Permissions']
            if ('SHT_PROGBITS' in item['Type'] and
                not 'SHF_ALLOC' in permission and
                not 'SHF_WRITE' in permission and
                not 'SHF_EXECINSTR' in permission and
                not 'SHT_REL' in item['Type']):
                name = self.get_name(item)
                if name.startswith('.debug_'):
                    res = True
        return res

    def get_section_type(self, section):
        '''Return section type.'''
        if self.is_code(section):
            return 'code'
        elif self.is_ro_data(section):
            return 'ro_data'
        elif self.is_rw_data(section):
            return 'rw_data'
        elif self.is_zi_data(section):
            return 'zi_data'
        elif self.is_debug(section):
            return 'debug'
        else:
            return None

    def is_lib_member(self, item):
        '''Return True if item is a member of some library, otherwise return
        False.
        '''
        origin = self.get_origin(item)
        if '(' in origin and origin.endswith(')'):
            return True
        else:
            return False

    def get_name(self, item):
        '''Return item->Name.'''
        if 'Name' in item:
            return item['Name']
        else:
            return ''

    def get_size(self, item):
        '''Return item->Size.'''
        if 'Size' in item:
            return item['Size']
        else:
            return 0

    def get_origin(self, item):
        '''Return item->Origin.'''
        if 'Origin' in item:
            return item['Origin']
        else:
            return None

    def get_grand_totals(self):
        '''Return grand_totals.'''
        if self.grand_totals:
            return self.grand_totals
        obj_totals = self.get_object_totals();
        lib_totals = self.get_library_totals();
        self.grand_totals = dict(obj_totals)
        for k in self.grand_totals:
            self.grand_totals[k] += lib_totals[k]
        return self.grand_totals

    def update_object_sizes(self, section, sec_type):
        '''Update size of objs, lib_objs and generated'''
        origin = self.get_origin(section)
        if origin is None:
            if not self.generated:
                self.generated = {
                    'code': 0,
                    'data': 0,
                    'ro_data': 0,
                    'rw_data': 0,
                    'zi_data': 0,
                    'debug': 0
                    }
            self.generated[sec_type] += self.get_size(section)
        elif self.is_lib_member(section):
            if not origin in self.lib_objs:
                self.lib_objs[origin] = {
                    'code': 0,
                    'data': 0,
                    'ro_data': 0,
                    'rw_data': 0,
                    'zi_data': 0,
                    'debug': 0
                    }
            self.lib_objs[origin][sec_type] += self.get_size(section)
        else:
            if not origin in self.objs:
                self.objs[origin] = {
                    'code': 0,
                    'data': 0,
                    'ro_data': 0,
                    'rw_data': 0,
                    'zi_data': 0,
                    'debug': 0
                    }
            self.objs[origin][sec_type] += self.get_size(section)

    def get_object_sizes(self):
        '''Populate objs, lib_objs and generated, and return objs.'''
        if self.objs:
            return self.objs
        for osection in self.yamlfile['OutputSections']:
            if self.is_code(osection):
                if osection['Contents'] is None:
                    self.update_object_sizes(osection, 'code')
                    continue
                for isection in osection['Contents']:
                    if self.is_code(isection):
                        self.update_object_sizes(isection, 'code')
                    elif (self.is_zi_data(isection) or
                          self.is_ro_data(isection) or
                          self.is_rw_data(isection)):
                        self.update_object_sizes(isection, 'data')
            else:
                sec_type = self.get_section_type(osection)
                if not sec_type:
                    continue
                if osection['Contents'] is None:
                    self.update_object_sizes(osection, sec_type)
                    continue
                for isection in osection['Contents']:
                    self.update_object_sizes(isection, sec_type)
        return self.objs

    def is_contents(self, item):
        '''Return True if item is contents, otherwise return False.'''
        return 'Contents' in item

    def get_permissions(self, item):
        '''Return item->Permissions.'''
        return item.get('Permissions', '')

    def get_address(self, item):
        '''Return item->Address.'''
        return item.get('Address', 0)

    def get_offset(self, item):
        '''Return item->Offset.'''
        return item.get('Offset', 0)

    def get_attr(self, item):
        '''Return item->attr.'''
        permission = self.get_permissions(item)
        if self.is_rw_data(item):
            return 'RW'
        elif self.is_ro_data(item):
            return 'RO'
        elif self.is_code(item) and not 'SHF_WRITE' in permission:
            return 'RO'
        elif self.is_zi_data(item):
            return 'RW'
        else:
            return '0'

    def get_type(self, item):
        '''Return item->Type.'''
        if self.is_code(item):
            return 'CODE'
        else:
            return 'DATA'

    def get_object_map_code(self, item):
        '''entry address code .'''
        i = 0
        self.object_map_code = {}
        for osection in self.yamlfile['OutputSections']:
            section_name = osection.get('Name', "")
            if self.is_contents(osection) and item == section_name:
                if osection['Contents'] is not None:
                    base = self.get_address(osection)
                    for isection in osection['Contents']:
                        addr = base + self.get_offset(isection)
                        size = self.get_size(isection)
                        type = self.get_type(isection)
                        attr = self.get_attr(isection)
                        e_section_name = self.get_name(isection)
                        origin = self.get_origin(isection)
                        permission = self.get_permissions(isection)

                        if (origin is not None and 'SHF_ALLOC' in permission and
                            item == section_name):
                            if self.object_map_code is None:
                                self.object_map_code[i] = {
                                    'addr': 0,
                                    'size': 0,
                                    'type': 0,
                                    'attr': 0,
                                    's_name': 0,
                                    'origin': 0
                                }
                            else:
                                self.object_map_code[i] = {
                                    'addr': addr,
                                    'size': size,
                                    'type': type,
                                    'attr': attr,
                                    's_name': e_section_name,
                                    'origin': origin
                                }
                                i += 1
                else:
                    return None
        return self.object_map_code

    def get_object_map(self):
        '''entry address code .'''
        entry_point = self.yamlfile['EntryAddress']
        print("Image Entry point: {0:#010x}".format(entry_point))
        for osection in self.yamlfile['LoadRegions']:
            if 'Sections' in osection and osection['Sections'] is not None:
                print("\n\n")
                print("Load Regions %s   Base: %08x, Size: %08x\n") % (osection['Name'],
                    osection['VirtualAddress'], osection['FileSize'] )
                for isection in osection['Sections']:
                    object_map = self.get_object_map_code(isection)
                    if object_map is not None:
                        print("\n Execution Region  %s \n") % (isection )
                        print("     Base Addr       Size         Type   Attr      E Section Name                        Object")
                        for k, v in object_map.items():
                            print("    {0:#010x}   {1:#011x}      {2:6s}  {3:6s}  {4:35s}     {5:11s}".format(
                                v['addr'], v['size'], v['type'],v['attr'], v['s_name'],
                                os.path.basename(v['origin'])))

    def get_reference(self):
        '''section reference code .'''
        for osection in self.yamlfile['CrossReference']:
            if osection is not None:
                refer_to = osection.get('Symbol', "")
                refer_file = osection.get('ReferencedBy', "")
                print(" %s refers to %s") % (refer_file, refer_to)

    def get_generated_sizes(self):
        '''Return generated.'''
        if not self.generated:
            self.get_object_sizes()
        if not self.generated:
            self.generated = {
                'code': 0,
                'data': 0,
                'ro_data': 0,
                'rw_data': 0,
                'zi_data': 0,
                'debug': 0
                }
        return self.generated

    def get_library_member_sizes(self):
        '''Return lib_objs.'''
        if not self.lib_objs:
            self.get_object_sizes()
        return self.lib_objs

    def get_library_sizes(self):
        '''Return libs.'''
        if self.libs:
            return self.libs
        lib_objs = self.get_library_member_sizes()
        for k, v in lib_objs.items():
            libname = get_library_name(k)
            if not libname in self.libs:
                self.libs[libname] = {
                    'code': 0,
                    'data': 0,
                    'ro_data': 0,
                    'rw_data': 0,
                    'zi_data': 0,
                    'debug': 0
                    }
            for t in v:
                self.libs[libname][t] += v[t]
        return self.libs

    def get_object_totals(self):
        '''Return obj_totals.'''
        if self.obj_totals:
            return self.obj_totals
        objs = self.get_object_sizes()
        generated = self.get_generated_sizes()
        self.obj_totals = dict(generated)
        for k, v in objs.items():
            for t in v:
                self.obj_totals[t] += v[t]
        return self.obj_totals

    def get_library_totals(self):
        '''Return lib_totals.'''
        if self.lib_totals:
            return self.lib_totals
        self.lib_totals = {
            'code': 0,
            'data': 0,
            'ro_data': 0,
            'rw_data': 0,
            'zi_data': 0,
            'debug': 0
            }
        lib_objs = self.get_library_member_sizes()
        for k, v in lib_objs.items():
            for t in v:
                self.lib_totals[t] += v[t]
        return self.lib_totals

    def get_unused(self):
        '''Return unsued sections.'''
        if self.unused:
            return self.unused
        for section in self.yamlfile['DiscardedSections']:
            self.unused[self.get_name(section)] = {
                'origin': self.get_origin(section),
                'size': self.get_size(section)
                }
        return self.unused

    def get_unused_symbols(self):
        '''Return unused symbols.'''
        if self.unused_symbols:
            return self.unused_symbols
        for section in self.yamlfile['DiscardedSections']:
            if 'Symbols' in section:
                for sym in section['Symbols']:
                    self.unused_symbols.append(sym['Symbol'])
        return self.unused_symbols

    def has_key_word(self, item):
        '''Return True if item is in file, otherwise return False.'''
        return item in self.yamlfile

def get_library_name(s):
    '''Return library name.'''
    return s[0:s.find("(")]


def get_library_member_name(s):
    '''Return library member name.'''
    return s[s.find("(")+1:s.find(")")]


def handle_map(yamlfile):
    '''Handle --map'''
    keyword1 = 'LoadRegions'
    keyword2 = 'OutputSections'
    keyword3 = 'EntryAddress'
    if (yamlfile.has_key_word(keyword1) and yamlfile.has_key_word(keyword2) and
        yamlfile.has_key_word(keyword3)):
        print("Memory Map of the image\n")
        yamlfile.get_object_map()
    else:
        print("No information about map load regions\n")


def handle_xref(yamlfile):
    '''Handle --xref'''
    keyword = 'CrossReference'
    if not yamlfile.has_key_word(keyword):
        print("No information about cross reference\n")
        return
    print("\n Section Cross References\n")
    yamlfile.get_reference()


def handle_sizes(yamlfile):
    '''Handle topic 'sizes' of --info.'''
    keyword = 'OutputSections'
    if not yamlfile.has_key_word(keyword):
        print("No information about size\n")
        return
    print("Image component sizes")
    objs = yamlfile.get_object_sizes()
    print("     Code (inc. data)   RO Data    RW Data    ZI Data      Debug   Object Name")
    for k, v in sorted(objs.items()):
        print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   {6}".format(
                v['code']+v['data'], v['data'], v['ro_data'], v['rw_data'],
                v['zi_data'], v['debug'], os.path.basename(k)))
    obj_totals = yamlfile.get_object_totals()
    print("   ----------------------------------------------------------------------")
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Object Totals".format(
            obj_totals['code']+obj_totals['data'], obj_totals['data'],
            obj_totals['ro_data'], obj_totals['rw_data'], obj_totals['zi_data'],
            obj_totals['debug']))
    generated = yamlfile.get_generated_sizes()
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   (incl. Generated)"
          .format(generated['code']+generated['data'], generated['data'],
                  generated['ro_data'], generated['rw_data'],
                  generated['zi_data'], generated['debug']))
    lib_objs = yamlfile.get_library_member_sizes()
    print("   ----------------------------------------------------------------------")
    print("     Code (inc. data)   RO Data    RW Data    ZI Data      Debug   Library Member Name")
    for k, v in lib_objs.items():
        print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   {6}".format(
                v['code']+v['data'], v['data'], v['ro_data'], v['rw_data'],
                v['zi_data'], v['debug'], get_library_member_name(k)))
    lib_totals = yamlfile.get_library_totals()
    print("   ----------------------------------------------------------------------")
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Library Totals".format(
            lib_totals['code']+lib_totals['data'], lib_totals['data'],
            lib_totals['ro_data'], lib_totals['rw_data'], lib_totals['zi_data'],
            lib_totals['debug']))
    libs = yamlfile.get_library_sizes()
    print("   ----------------------------------------------------------------------")
    print("     Code (inc. data)   RO Data    RW Data    ZI Data      Debug   Library Name")
    for k, v in libs.items():
        print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   {6}".format(
                v['code']+v['data'], v['data'], v['ro_data'], v['rw_data'],
                v['zi_data'], v['debug'], os.path.basename(k)))
    print("   ----------------------------------------------------------------------")
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Library Totals".format(
            lib_totals['code']+lib_totals['data'], lib_totals['data'],
            lib_totals['ro_data'], lib_totals['rw_data'], lib_totals['zi_data'],
            lib_totals['debug']))
    print("   ----------------------------------------------------------------------")
    grand_totals = yamlfile.get_grand_totals()
    print("=============================================================================")
    print("     Code (inc. data)   RO Data    RW Data    ZI Data      Debug")
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Grand Totals".format(
            grand_totals['code']+grand_totals['data'], grand_totals['data'],
            grand_totals['ro_data'], grand_totals['rw_data'],
            grand_totals['zi_data'], grand_totals['debug']))
    print("=============================================================================")
    total_ro_size = (grand_totals['code'] + grand_totals['data']
                     + grand_totals['ro_data'])
    print("   Total RO  Size (Code + RO Data)          {0:11d} ({1:7.2f}kB)"
          .format(total_ro_size,total_ro_size/1024.0))
    total_rw_size = grand_totals['rw_data'] + grand_totals['zi_data']
    print("   Total RW  Size (RW Data + ZI Data)       {0:11d} ({1:7.2f}kB)"
          .format(total_rw_size,total_rw_size/1024.0))
    total_rom_size = (grand_totals['code'] + grand_totals['data']
                      + grand_totals['ro_data'] + grand_totals['rw_data'])
    print("   Total ROM Size (Code + RO Data + RW Data){0:11d} ({1:7.2f}kB)"
          .format(total_rom_size,total_rom_size/1024.0))
    print("=============================================================================")


def handle_summarysizes(yamlfile):
    '''Handle topic 'summarysizes' of --info.'''
    keyword = 'OutputSections'
    if not yamlfile.has_key_word(keyword):
        print("No information about summary sizes\n")
        return
    grand_totals = yamlfile.get_grand_totals()
    print("Program Size: Code={0:d} RO-data={1:d} RW-data={2:d} ZI-data={3:d}"
          .format(grand_totals['code']+grand_totals['data'],
                  grand_totals['ro_data'], grand_totals['rw_data'],
                  grand_totals['zi_data']))


def handle_totals(yamlfile):
    '''Handle topic 'totals' of --info.'''
    keyword = 'OutputSections'
    if not yamlfile.has_key_word(keyword):
        print("No information about totals\n")
        return
    print("Image component sizes")
    obj_totals = yamlfile.get_object_totals()
    print("     Code (inc. data)   RO Data    RW Data    ZI Data      Debug")
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Object Totals".format(
            obj_totals['code']+obj_totals['data'], obj_totals['data'],
            obj_totals['ro_data'], obj_totals['rw_data'], obj_totals['zi_data'],
            obj_totals['debug']))
    generated = yamlfile.get_generated_sizes()
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   (incl. Generated)"
          .format(generated['code']+generated['data'], generated['data'],
                  generated['ro_data'], generated['rw_data'],
                  generated['zi_data'], generated['debug']))
    lib_totals = yamlfile.get_library_totals()
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Library Totals".format(
            lib_totals['code']+lib_totals['data'], lib_totals['data'],
            lib_totals['ro_data'], lib_totals['rw_data'], lib_totals['zi_data'],
            lib_totals['debug']))
    libs = yamlfile.get_library_sizes()
    grand_totals = yamlfile.get_grand_totals()
    print("=============================================================================")
    print("     Code (inc. data)   RO Data    RW Data    ZI Data      Debug")
    print("{0:9d}{1:11d}{2:11d}{3:11d}{4:11d}{5:11d}   Grand Totals".format(
            grand_totals['code']+grand_totals['data'], grand_totals['data'],
            grand_totals['ro_data'], grand_totals['rw_data'],
            grand_totals['zi_data'], grand_totals['debug']))
    print("=============================================================================")
    total_ro_size = (grand_totals['code'] + grand_totals['data']
                     + grand_totals['ro_data'])
    print("   Total RO  Size (Code + RO Data)          {0:11d} ({1:7.2f}kB)"
          .format(total_ro_size,total_ro_size/1024.0))
    total_rw_size = grand_totals['rw_data'] + grand_totals['zi_data']
    print("   Total RW  Size (RW Data + ZI Data)       {0:11d} ({1:7.2f}kB)"
          .format(total_rw_size,total_rw_size/1024.0))
    total_rom_size = (grand_totals['code'] + grand_totals['data']
                      + grand_totals['ro_data'] + grand_totals['rw_data'])
    print("   Total ROM Size (Code + RO Data + RW Data){0:11d} ({1:7.2f}kB)"
          .format(total_rom_size,total_rom_size/1024.0))
    print("=============================================================================")


def handle_architecture(yamlfile):
    '''Handle topic 'architecture' of --info.'''
    print("Architecture: " + yamlfile.get_architecture())
    print("Emulation: " + yamlfile.get_emulation())
    print("AddressSize: " + yamlfile.get_address_size())


def handle_unused(yamlfile):
    '''Handle topic 'unused' of --info.'''
    keyword = 'DiscardedSections'
    if not yamlfile.has_key_word(keyword):
        print("No information about unused")
        return
    print("Removing Unused input sections from the image.")
    unused = yamlfile.get_unused()
    total_size = 0
    for k,v in unused.items():
        total_size += v['size']
        print("  Removing {0}({1}), ({2:d} bytes).".format(
                os.path.basename(v['origin']), k, v['size']))
    print("{0:d} unused section(s) (total {1:d} bytes) removed from the image."
          .format(len(unused), total_size))


def handle_unusedsymbols(yamlfile):
    '''Handle topic 'unusedsymbols' of --info.'''
    keyword = 'DiscardedSections'
    if not yamlfile.has_key_word(keyword):
        print("No information about unusedsymbols")
        return
    print("Removing Unused input sections from the image.")
    unused_symbols = yamlfile.get_unused_symbols()
    for sym in unused_symbols:
        print("       Removing symbol {0}.".format(sym))
    print("{0:d} unused symbol(s) removed from the image.".format(
            len(unused_symbols)))


def handle_info(yaml_file, info):
    '''Handle --info=topic[,topic,...].'''
    usupported_topic = ""
    info_list = info.split(',')
    for topic in info_list:
        if topic == 'sizes':
            handle_sizes(yaml_file)
        elif topic == 'summarysizes':
            handle_summarysizes(yaml_file)
        elif topic == 'totals':
            handle_totals(yaml_file)
        elif topic == 'architecture':
            handle_architecture(yaml_file)
        elif topic == 'unused':
            handle_unused(yaml_file)
        elif topic == 'unusedsymbols':
            handle_unusedsymbols(yaml_file)
        else:
            usupported_topic += topic
            print('Unsupported topic: ' + topic)


def main():
    parser = argparse.ArgumentParser(description='YAML parser', usage=
                                     '%(prog)s [-h] [--info=topic[,topic,...]] [--map] [--list=file] [--xref] YAML_file',
                                     formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('yaml_file', help='The input YAML file.')
    parser.add_argument('--info', help='''List misc. information about image.
 Available topics: (separate multiple topics with comma)
  architecture  List architecture.
  sizes         List code and data sizes for objects in image.
  summarysizes  List code and data sizes of the image.
  totals        List total sizes of all objects in image.
  unused        List sections eliminated from the image.
  unusedsymbols List symbols eliminated from the image.''')
    parser.add_argument('--map', help='Display memory map of image.',
                        action='store_true')
    parser.add_argument('--list', help='Redirect output to a file.')
    parser.add_argument('--xref', help=
                        'List all cross-references between input sections.',
                        action='store_true')
    args = parser.parse_args()
    with open(args.yaml_file, 'r') as f:
        yamlfile = YAMLFile(f)

    if args.list:
        output_file = open(args.list, 'w')
        sys.stdout = output_file

    if args.info:
        handle_info(yamlfile, args.info)

    if args.map:
        handle_map(yamlfile)

    if args.xref:
        handle_xref(yamlfile)

    if args.list:
        output_file.close()

if __name__ == '__main__':
    main()
