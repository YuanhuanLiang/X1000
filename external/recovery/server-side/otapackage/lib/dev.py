import sys
import re
import os
import xml.etree.cElementTree as et
from otapackage.lib import base, ini, log, image
from otapackage import config

class Device(object):

    printer = None
    partition_cnt = 0
    @base.struct('devinfo', 'partition')
    def __init__(self, *value):
        pass

    class Devinfo(object):

        @base.struct('devtype', 'devsize')
        def __init__(self, *value):
            pass

        def get_nand_reserved_size(self):
            pebs = self.devsize / config.nandflash_block_size
            reserved_unit = config.ubi_mtd_ubi_beb_limit_per1024
            denum = 1024
            quot = pebs / denum
            rem = pebs % denum
            reserved = (quot * (reserved_unit)) + \
                ((rem * (reserved_unit)) / (denum))
            if reserved < self.devsize:
                reserved += 1
            return reserved * config.nandflash_block_size

    class Partition(object):

        @base.struct('name', 'offset', 'size', 'type')
        def __init__(self, *value):
            pass

        def judge(self):
            Device.printer.debug("judge on partition \'%s\'" %(self.name))
            if (self.offset < 0) or (self.size <= 0):
                Device.printer.error(
                    'error while getting offset or size on partition \'%s\'' % (
                        self.name))
                return False
            return True

        def get_available_mapped_content_size(self, ftype):
            pagesize = config.nandflash_page_size
            blksize = pagesize * config.nandflash_pages_per_block
            size = self.size
            if ftype == 'normal':
                return size
            elif ftype == 'ubifs':
                ubi_reserved_blks = config.ubi_layout_volume_ebs + \
                    config.ubi_wl_reserved_pebs + config.ubi_eba_reserved_pebs
                ubi_reserved = ubi_reserved_blks * blksize
                return size - ubi_reserved
            elif ftype == 'jffs2':
                return size
            elif ftype == 'cramfs':
                return size
            elif ftype == 'yaffs2':
                yaffs_max = size / pagesize * \
                    (pagesize + config.yaffs2_tagsize_per_page)
                return yaffs_max
            else:
                return -1

        def is_mmaped_in(self, start, size):
            pstart = self.offset
            pend = self.size + pstart
            if start >= pstart and start < pend:
                return True
            return False

        def generate_config(self, element):
            Device.printer.debug("generate partition[\'%s\'] config" %(self.name))
            eroot = et.SubElement(element, 'item')
            element_name = et.SubElement(eroot, 'name')
            element_name.attrib = {"type": config.xml_data_type_string}
            element_name.text = self.name
            element_type = et.SubElement(eroot, 'offset')
            element_type.text = '0x%x' % self.offset
            element_offset = et.SubElement(eroot, 'size')
            element_offset.text = '0x%x' % self.size
            element_size = et.SubElement(eroot, 'blockname')
            element_size.attrib = {"type": config.xml_data_type_string}
            element_size.text = self.type
            return eroot

    # interact with image info, persume image is mapped in partition
    def judge_partititon_mapped(self, map_start, map_size, ftype):
        partitions = self.partition
        devinfo = self.devinfo

        for i in range(0, len(partitions)):
            if partitions[i].is_mmaped_in(map_start, map_size):
                break

        if i == (len(partitions) + 1):
            self.printer.error(
                'can not get image at offset 0x%x' % (map_start))
            return False

        mapped_max = partitions[i].get_available_mapped_content_size(ftype)
        if mapped_max <= 0:
            self.printer.error(
                '%s: can not get available map size at partition 0x%x' % (
                    partitions[i].offset))

            return False

        if devinfo.devtype == 'nand' and ftype == 'ubifs':
            mapped_max -= devinfo.get_nand_reserved_size()

        if (map_start + map_size) > (partitions[i].offset + mapped_max):
            self.printer.error(
                'image 0x%x can not mapped in partition 0x%x' % (
                    map_start, partitions[i].offset))
            return False
        return True

    @classmethod
    def get_object_from_file(cls):
        cls.printer = log.Logger.get_logger(config.log_console)
        cls.printer.debug("dev customization parser is starting")
        pathdev = os.path.join(
            config.Config.get_image_cfg_path(),
            "%s" %(config.Config.make_customer_files_fullname(config.partition_file)))
        ini_parser = ini.ParserIni(pathdev)
        section = 'storageinfo'
        devtype = ini_parser.get(section, 'mediumtype')
        devsize = ini_parser.get(section, 'capacity')
        if devtype == '' or devsize == '':
            cls.printer.error('section %s is wrong or lost items' % (section))
            return None
        devinfo = cls.Devinfo(devtype, base.human2bytes(devsize))
        plist = ini_parser.get_options('partition')
        partitions = []
        section = 'partition'
        cls.partition_cnt = len(plist)
        for i in range(0, len(plist)):
            item = ini_parser.get(section, plist[i])
            if plist[i] != "item%d"%(i+1):
                 cls.printer.error(
                        '%s is named wrong' % (plist[i]))
                 return None
            fields = item.split(',')
            for field in fields:
                if field == '':
                    cls.printer.error(
                        'item %s[%d] has empty field' % (section, i))
                    return None
            p = cls.Partition(fields[0], base.str2int(fields[1]),
                              base.str2int(fields[2]), fields[3])
            partitions.append(p)
        device = cls(devinfo, partitions)
        if not device.judge():
            return None
        return device

    def judge(self):
        self.printer.debug("dev judgement is starting")
        if self.devinfo.devtype not in config.device_types:
            self.printer.error("devtype \'%s\' is not in %s", 
                self.devinfo.devtype, config.device_types)
            return False
        totalsize = self.devinfo.devsize
        if totalsize == -1:
            self.printer.error("devsize can not be recognised")
            return False

        for i in range(0, len(self.partition)):

            p = self.partition[i]
            if not p.judge():
                self.printer.error(
                    'error while judging partition item %d' % (i))
                return False
            co = p.offset
            cs = p.size
            if co >= totalsize or cs > totalsize:
                self.printer.error(
                    'partition offset 0x%x or size 0x%x is overlow' % (co, cs))
                return False
            if (co + cs) > totalsize:
                self.printer.error(
                    '''partition offset 0x%x plus size 0x%x is overlow''' % (co, cs))
                return False

            if (i < len(self.partition)-1):
                pnext = self.partition[i+1]
                if not pnext.judge():
                    self.printer.error(
                        'error while judging partition item %d' % (i+1))
                    return False
                no = pnext.offset
                if (co + cs) > no:
                    self.printer.error(
                        '''partition offset 0x%x plus size 0x%x is overlap with the next item''' % (co, cs))
                    return False
        # print 'create device success'
        return True

    def generate(self):
        self.printer.debug("dev generation is starting")
        default_config_dir = "%s/%s/%s" % (config.Config.get_outputdir_path(),
                                        config.Config.get_customer_files_suffix(),
                                        config.output_pack_config_dir)

        default_config = "%s/%s" % (default_config_dir,
                                    config.output_partition_name)

        if not os.path.exists(default_config_dir):
            os.makedirs(default_config_dir)

        # devinfo_root = et.Element('devinfo')
        # devinfo_type_root = et.SubElement(devinfo_root, 'type')
        # devinfo_type_root.attrib = {"type": config.xml_data_type_string}
        # devinfo_type_root.text = self.devinfo.devtype
        # devinfo_size_root = et.SubElement(devinfo_root, 'capacity')
        # devinfo_size_root.text = '0x%x' % self.devinfo.devsize

        partition_root = et.Element('partition')
        partition_root.attrib = {"count": "%s" %self.partition_cnt}
        for partition in self.partition:
            partition.generate_config(partition_root)

        root = et.Element('device')
        root.attrib =  {"type": "%s" %self.devinfo.devtype, "capacity":"0x%x" %self.devinfo.devsize}
        # root_element_list = [devinfo_root,partition_root]
        root_element_list = [partition_root]
        root.extend(root_element_list)
        tree = et.ElementTree(root)
        path = default_config
        target = open(path, 'w')
        tree.write(target, encoding=config.xml_encoding,
                   xml_declaration=config.xml_declaration)
        return True
