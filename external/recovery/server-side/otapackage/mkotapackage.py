import os
import sys
import shutil
import argparse
import xml.etree.cElementTree as et
from otapackage.lib import base, log, image, dev
from otapackage import config


class Maker(object):

    printer = None
    customer_types = []

    def __init__(self):
        pass

    def get_customer_effective_types(self):
        partition = []
        customize = []
        images = []
        effectives = []

        path = config.Config.get_image_cfg_path()
        for parent, dirnames, filenames in os.walk(path):
            for filename in filenames:
                if not filename.endswith(config.configuration_file_suffix):
                    continue
                filename = filename[0:-len(config.configuration_file_suffix)]
                f = "%s_" %(config.partition_file)
                if filename.startswith(f):
                    filename = filename[len(f):]
                    if filename not in config.device_types:
                        continue
                    partition.append(filename)
                f = "%s_" %(config.customize_file)
                if filename.startswith(f):
                    filename = filename[len(f):]
                    if filename not in config.device_types:
                        continue
                    customize.append(filename)

        path = config.Config.get_image_path()
        for parent, dirnames, filenames in os.walk(path):
            for dirname in dirnames:
                if dirname not in config.device_types:
                    continue
                images.append(dirname)

        tmp = []
        for img in images:
            for cz in customize:
                if img == cz:
                    tmp.append(img)

        for cb in tmp:
            for fc in partition:
                if cb == fc:
                    effectives.append(cb)

        i = 0
        for difftype in config.device_types_diff:
            for e in effectives:
                if difftype == e:
                    i += 1
        if i == len(config.device_types_diff):
            self.printer.error("%s cannot exist on the same time", 
                    config.device_types_diff)
            return []

        self.printer.debug(effectives)
        return effectives

    def start(self):

        self.customer_types = self.get_customer_effective_types()
        if not self.customer_types:
            self.printer.error("cannot get any customer files")
            os._exit(1)

        for customer_type in self.customer_types:
            print "pack start on \'%s\'" %(customer_type)
            config.Config.set_customer_files_suffix(customer_type)
            self.dev = dev.Device.get_object_from_file()
            if not self.dev:
                self.printer.error("parsing file %s" %
                                   (config.partition_file_name))
                os._exit(1)

            self.image = image.Image.get_object_from_file()
            if not self.image:
                self.printer.error("parsing file %s" %
                                   (config.customize_file_name))
                os._exit(1)

            if not self.judge():
                self.printer.error("judging mapping image")
                os._exit(1)

            self.image.generate()

            self.dev.generate()
            self.pack()

        self.generate(self.customer_types)
        os._exit(0)

    def generate(self, list_devs):

        root_element_list = []
        for i in range(0, len(list_devs)):
            branch = et.Element('device')
            branch.attrib = {"type": config.xml_data_type_string}
            branch.text =  list_devs[i]
            root_element_list.append(branch)
            
        root = et.Element('global')
        root.extend(root_element_list)
        tree = et.ElementTree(root)

        path = "%s/%s" %(config.Config.get_outputdir_path(),
                    config.output_global_config_name)
        target = open(path, 'w')
        tree.write(target, encoding=config.xml_encoding,
                   xml_declaration=config.xml_declaration)

    def pack(self):
        packagecnt = image.Image.get_image_total_cnt()
        self.printer.debug("Total %d packages will be created" %
                           (packagecnt))
        if config.signature_flag:
            cipher_lib_path = os.path.realpath(
                "%s/%s" % (config.signature_home, config.signature_cipher_lib))
            # keys_public_path = os.path.realpath(
            #     config.signature_rsa_public_key)
            # keys_private_path = os.path.realpath(
            #     config.signature_rsa_private_key)
            keys_public_path = os.path.realpath(
                config.Config.get_public_key())
            keys_private_path = os.path.realpath(
                config.Config.get_private_key())
        orgdir = os.getcwd()

        os.chdir("%s/%s" %(config.Config.get_outputdir_path(),
                config.Config.get_customer_files_suffix()))
        for i in range(0, packagecnt+1):
            output_package = ("%s%03d") % (config.output_package_name, i)
            output_package_unencrypted = ("%s.unencrypted.zip" % (
                output_package))
            output_package_encrypted = ("%s.zip" % (output_package))
            caller_zip = "zip -r %s %s" % (
                output_package_unencrypted, output_package)
            os.system(caller_zip)
            if config.signature_flag:
                caller_signature = "java -jar -Xms512M -Xmx1024M %s -w %s %s %s %s" % (
                    cipher_lib_path, keys_public_path, keys_private_path,
                    output_package_unencrypted, output_package_encrypted)
                os.system(caller_signature)

            os.remove(output_package_unencrypted)
            shutil.rmtree(output_package)
        os.chdir(orgdir)
        return True

    # problem occured when multiple filesystem image is in one partittion
    # for example:  mtdblock3 <== (ubifs + others file system)
    def judge(self):
        dev = self.dev
        imageinfos = self.image.imageinfo
        for img in imageinfos:
            if not dev.judge_partititon_mapped(img.offset, img.size, img.type):
                self.printer.error("mapping image into partition")
                return False
        return True

    def cmdline_parse(self):
        parser = argparse.ArgumentParser()
        help = '''The Image path is where you place the images'''
        parser.add_argument('-i', '--imgpath', help=help)
        help = '''The output path is where you place the generated packages'''
        parser.add_argument('-o', '--output', help=help)
        help = '''The slice size is the length you pointed for slice updatemode.
                  Multiple is interger, base is %dMB.
                  Default is %dMB''' % (
            config.slicebase/1024/1024, config.slicesize/config.slicebase)
        parser.add_argument('-s', '--slicesize', help=help, type=int)
        help = '''Print debug information'''
        parser.add_argument('-v', '--verbose', help=help, action='store_true')

        if config.signature_flag:
            help = '''public key path'''
            parser.add_argument('--publickey', help=help)
            help = '''private key path'''
            parser.add_argument('--privatekey', help=help)

        args = parser.parse_args()
        if args.imgpath:
            config.Config.set_image_path(args.imgpath)
        if args.output:
            config.Config.set_outputdir_path(args.output)
        if args.slicesize:
            config.Config.set_slicesize(args.slicesize)
        if config.signature_flag:
            if args.publickey:
                config.Config.set_public_key(args.publickey)
            if args.privatekey:
                config.Config.set_private_key(args.privatekey)

        self.printer = log.Logger.get_logger(config.log_console)
        if args.verbose == True:
            log.Logger.set_level(config.log_console, 'd')

        self.printer.debug("command line parser")
        self.printer.debug("image configuration file path: %s" %
                           (config.Config.get_image_cfg_path()))
        self.printer.debug("image file path: %s" %
                           (config.Config.get_image_path()))
        self.printer.debug("output directory path: %s" %
                           (config.Config.get_outputdir_path()))
        self.printer.debug("slice size: %d" % (config.Config.get_slicesize()))
        self.printer.debug("public key path: %s" %
                           (config.Config.get_public_key()))
        self.printer.debug("private key path: %s" %
                           (config.Config.get_private_key()))
        return self


def main():
    Maker().cmdline_parse().start()

if __name__ == '__main__':
    main()
    pass
