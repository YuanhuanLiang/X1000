#!/usr/bin/python
import os
import sys
import re
import argparse
from otapackage import config
from otapackage.lib import base
from otapackage.customer.config import *

class DeviceConfig(object):
    devfile = config.partition_file
    filesuffix = config.configuration_file_suffix
    devtypes = []
    devsizes = []
    devpartitions = []
    medium_current = ""
    for k in sorted(local):
        if k[0:3] != local['search_prefix']:
            continue
        if re.search('_tag', k):
            devtypes.append(local[k])
        if re.search('_device_size', k):
            devsizes.append(local[k])
        if re.search('_partition', k):
            devpartitions.append(local[k])
    assert (len(devtypes) == len(devsizes)) and  (len(devtypes) == len(devpartitions)) \
                ,"list length of devtypes, devsizes and devpartitions must equivalent"

    devs = zip(devtypes, devsizes, devpartitions)

    local = locals()

    def __init__(self, dev):
        self.dev = dev
        pass

    @classmethod
    def get_default(cls, type):
        assert type in cls.devtypes, "device type '%s' is not in %s" %(devtype, cls.devtypes)
        for devtype, devsize, devpartition in cls.devs:
            if devtype == type:
                dev = []
                dev.append(devtype)
                dev.append(devsize)
                dev.append(devpartition)
                return cls(dev) 
        return None

    def generate_output(self):
        buf = "[storageinfo]\n"
        buf += "mediumtype=%s\n" %(self.dev[0])
        buf += "capacity=%s\n" %(self.dev[1])
        buf += "[partition]\n"
        for row in range(0, len(self.dev[2])):
            s = 'item%d='%(row+1)
            for col in range(0, len(self.dev[2][row])):
                s += self.dev[2][row][col]
                if col == (len(self.dev[2][row])-1):
                    s += '\n'
                else:
                    s += ','
            buf += s
        # print buf
        return buf

    def create_device_descript(self, filepath):
        buf = ''
        filename = '%s/%s_%s%s'%(filepath, self.devfile, self.medium_current, self.filesuffix)
        fd = open(filename,'w')
        buf = self.generate_output()
        fd.write(buf)
        fd.close()
        return True

    @classmethod
    def cmdline_parse(cls):
        parser = argparse.ArgumentParser()
        help = 'The medium type to update on. Must be one in %s%s' %("", tuple(cls.devtypes))
        parser.add_argument('mediumtype', help=help)
        args = parser.parse_args()
        if args.mediumtype not in cls.devtypes:
            print '%s: error: medium type %s is not one in %s' %(__file__, args.mediumtype, tuple(cls.devtypes))
            os._exit(0)
        cls.medium_current = args.mediumtype
        return args


def main():
    args = DeviceConfig.cmdline_parse()
    filepath = config.customer_path
    DeviceConfig.get_default(args.mediumtype).create_device_descript(filepath)

if __name__ == '__main__':
    main()