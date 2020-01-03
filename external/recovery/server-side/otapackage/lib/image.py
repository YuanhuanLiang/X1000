import os
import sys
import shutil
import xml.etree.cElementTree as et
from otapackage.lib import base, ini, file, log
from otapackage import config


class Image(object):
    imagetypes = config.img_types
    updatemodes = config.updatemodes
    devctls = config.devctls
    imagesum = config.output_pack_config_index
    printer = None

    class UpdateMode(object):

        @base.struct('type', 'size', 'count')
        def __init__(self, *value):
            pass

        @classmethod
        def get_slice_size(cls, imagetype, size):
            minimum_slice = config.nandflash_block_size
            multiple = size / minimum_slice
            if imagetype == 'ubifs':
                minimum_slice = config.ubi_leb_size
            elif imagetype == 'yaffs2':
                minimum_slice = config.yaffs2_block_size
            slicesize = minimum_slice * multiple
            return slicesize

        @classmethod
        def get_default(cls, imagename, imagetype,
                        updatemode, imagesize, slicesize):
            # imagesize, imagetype, updatemode,
            if updatemode not in Image.updatemodes:
                Image.printer.error(
                    '''error while getting 
                    updatemode from image %s''' % (imagename))
                return None
            updatetype = config.e_updatemodes['%s' % updatemode]
            size = 0
            count = 0
            if updatemode == Image.updatemodes[1]:
                size = cls.get_slice_size(imagetype, slicesize)
                count = (imagesize + size - 1) / size
            return cls(updatetype, size, count)

    class Imageinfo(object):

        @base.struct('name', 'size', 'type', 'offset', 'updatemode')
        def __init__(self, *value):
            pass

        # pack images to packages, generate config info simultaneously
        def generate(self, element):
            Image.printer.debug(
                "imageinfo[\'%s\'] generation is starting" % (
                    self.name))
            path_src_image = os.path.join(
                config.Config.get_image_path(),
                "%s/%s" %(config.Config.get_customer_files_suffix(), self.name))
            path_dst_image_dir = "%s/%s" %(
                    config.Config.get_outputdir_path(), config.Config.get_customer_files_suffix())
            if element == None:
                Image.printer.error('error while getting parameter element')
                return None

            count = 1
            if self.updatemode.size > 0:
                count = file.split(
                    path_src_image, path_dst_image_dir,
                    self.updatemode.size, self.generate_process)
                if count != self.updatemode.count:
                    Image.printer.error('''error:split file count %d is 
                        not equal with preset value %d''' % (
                        count, self.updatemode.count))
                    return None
            else:
                path_dst_image = "%s/%s" % (path_dst_image_dir, self.name)
                if not os.path.exists(path_dst_image_dir):
                    os.makedirs(path_dst_image_dir)
                shutil.copy(path_src_image, path_dst_image_dir)
                self.generate_process(path_dst_image)

            eroot = self.generate_config(element)
            return eroot

        def generate_config(self, element):
            Image.printer.debug(
                "generate imageinfo[\'%s\'] config" % (self.name))
            eroot = et.SubElement(element, 'image')
            element_name = et.SubElement(eroot, 'name')
            element_name.attrib = {"type": config.xml_data_type_string}
            element_name.text = self.name
            element_type = et.SubElement(eroot, 'type')
            element_type.attrib = {"type": config.xml_data_type_string}
            element_type.text = self.type
            element_offset = et.SubElement(eroot, 'offset')
            element_offset.text = '0x%x' % self.offset
            element_size = et.SubElement(eroot, 'size')
            element_size.text = '%d' % self.size
            element_mode = et.SubElement(eroot, 'updatemode')
            element_mode_type = et.SubElement(element_mode, 'type')
            element_mode_type.attrib = {"type": config.xml_data_type_integer}
            element_mode_type.text = '0x%x' % self.updatemode.type
            if self.updatemode.type == config.e_updatemodes['slice']:
                element_mode_size = et.SubElement(element_mode, 'chunksize')
                element_mode_size.attrib = {"type": config.xml_data_type_integer}
                element_mode_size.text = '%d' % self.updatemode.size
                element_mode_count = et.SubElement(element_mode, 'chunkcount')
                element_mode_count.attrib = {"type": config.xml_data_type_integer}
                element_mode_count.text = '%d' % self.updatemode.count
            return eroot

        def generate_process(self, imagename):
            imagepath = os.path.split(os.path.realpath(imagename))[0]
            Image.imagesum += 1
            packdir = os.path.join(
                imagepath, '%s%03d' % (
                    config.output_package_name, Image.imagesum))
            if not os.path.exists(packdir):
                os.makedirs(packdir)
            shutil.move(imagename, packdir)

        def judge(self):
            Image.printer.debug("judge on image \'%s\'" % (self.name))
            if self.type not in Image.imagetypes:
                Image.printer.error('%s with type %s wrong' %
                                   (self.name, self.type))
                return False
            if self.offset < 0:
                Image.printer.error('%s offset is not number' % (self.name))
                return False
            return True

    @base.struct('mediumtype', 'imgcnt', 'devctl', 'imageinfo')
    def __init__(self, *value):
        pass

    @classmethod
    def get_image_total_cnt(cls):
        return cls.imagesum - config.output_pack_config_index

    @classmethod
    def get_image_size(cls, imagename):
        imgfull = "%s/%s" %(config.Config.get_customer_files_suffix() ,imagename)
        apath = os.path.join(
            config.Config.get_image_path(), ('%s' % (imgfull)))
        if not file.is_readable(apath):
            cls.printer.error('file %s is not readable' % (apath))
            return 0
        imagesize = os.path.getsize(apath)
        if not imagesize:
            cls.printer.error(
                'error while getting file \'%s\' size' % (apath))
            return 0
        return imagesize

    @classmethod
    def get_object_from_file(cls):
        cls.printer = log.Logger.get_logger(config.log_console)
        cls.printer.debug(
            "image customization parser is starting")
        imagecfg = '%s/%s' % (config.Config.get_image_cfg_path(),
                              config.Config.make_customer_files_fullname(config.customize_file))
        ini_parser = ini.ParserIni(imagecfg)
        mediumtype =  ini_parser.get('update', 'mediumtype')
        if mediumtype == '':
            cls.printer.error(
                    'label update  or mediumtype is lost')
            return None
        imgcnt = ini_parser.get('update', 'imgcnt')
        if imgcnt == '':
            cls.printer.error(
                    'label update or imgcnt is lost')
            return None
        if not imgcnt.isdigit():
            cls.printer.error(
                    "imgcnt must be number but not character")
            return None
        imgcnt = base.str2int(imgcnt)
        devctl = ini_parser.get('update', 'devctl')
        devctl = 0 if not devctl else base.str2int(devctl)

        imageinfos = []
        for i in range(1, imgcnt+1):
            section_name = 'image%d' % i
            if not ini_parser.get_options(section_name):
                return None
            name = ini_parser.get(section_name, 'name')
            imgtype = ini_parser.get(section_name, 'type')
            offset = ini_parser.get(section_name, 'offset')
            updatemode = ini_parser.get(section_name, 'updatemode')
            if base.is_one_empty(zip(name, imgtype, offset, updatemode)):
                return None
            imgsize = cls.get_image_size(name)
            if imgsize == 0:
                return None

            updateinfo = Image.UpdateMode.get_default(
                name, imgtype, updatemode,
                imgsize, config.Config.get_slicesize())
            if not updateinfo:
                cls.printer.error(
                    'error while creating updateinfo on image %s' % (name))
                return None
            imageinfo = cls.Imageinfo(
                name, imgsize, imgtype, base.str2int(offset), updateinfo)
            imageinfos.append(imageinfo)
        image = cls(mediumtype, imgcnt, devctl, imageinfos)
        if not image.judge():
            return None
        cls.imagesum = config.output_pack_config_index
        return image

    def judge(self):
        self.printer.debug("image judgement is starting")
        if self.mediumtype not in config.device_types:
            self.printer.error('mediumtype %s is not in %s' % (self.mediumtype, config.device_types))
            return False
        if self.imgcnt != len(self.imageinfo):
            return False
        hit = 0
        for k in self.devctls:
            if self.devctl == self.devctls[k]:
                hit = 1
                break
        if hit == 0:
            self.printer.error('devctl=%d is not supported' % (self.devctl))
            return False
        for i in range(0, len(self.imageinfo)):
            self.printer.debug("judgement step%d" %(i))
            p = self.imageinfo[i]
            if not p.judge():
                self.printer.error('error while parsing imageinfo%d' % (i+1))
                return False

            co = p.offset
            cs = p.size
            if i < (len(self.imageinfo)-1):
                pnext = self.imageinfo[i+1]
                if not pnext.judge():
                    self.printer.error(
                        'error while parsing imageinfo%d' % (i+1))
                    return False
                no = pnext.offset
                if (co + cs) > no:
                    self.printer.error(
                        'error on image%d field \'offset\'' % (i+1))
                    return False
        # print 'create image success'
        return True

    def generate(self):
        self.printer.debug("image generation is starting")
        path = "%s/%s" % (config.Config.get_outputdir_path(),
                    config.Config.get_customer_files_suffix())

        if os.path.exists(path):
            shutil.rmtree(path)

        default_config_dir = "%s/%s/%s" % (
            config.Config.get_outputdir_path(),
            config.Config.get_customer_files_suffix(),
            config.output_pack_config_dir)
        default_config = "%s/%s" % (default_config_dir,
                                    config.output_config_name)
        if not os.path.exists(default_config_dir):
            os.makedirs(default_config_dir)

        # devctl_root = et.Element('devctl')
        # devctl_root.attrib = {"type": config.xml_data_type_integer}
        # devctl_root.text = "%d" % (self.devctl)
        imglist_root = et.Element('imagelist')
        imglist_root.attrib = {"count": "%d" % self.imgcnt}

        for imageinfo in self.imageinfo:
            imginfo_root = imageinfo.generate(imglist_root)

        root = et.Element('update')
        root_element_list = [imglist_root]
        root.attrib =  {"devtype": "%s" %config.Config.get_customer_files_suffix(),
                              "devcontrol":"0x%x" %self.devctl}
        root.extend(root_element_list)
        tree = et.ElementTree(root)
        path = default_config
        target = open(path, 'w')
        tree.write(target, encoding=config.xml_encoding,
                   xml_declaration=config.xml_declaration)
        return True
