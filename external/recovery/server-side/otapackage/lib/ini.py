import ConfigParser


class ParserIni:

    def __init__(self, path):
        self.path = path
        self.cf = ConfigParser.ConfigParser()
        self.cf.read(self.path)

    def get_sections(self):
        return self.cf.sections()

    def get_options(self, section_name):
        return self.cf.options(section_name)

    def get_items(self, item_name):
        return self.cf.items(item_name)

    def get(self, field, key):
        result = ""
        try:
            result = self.cf.get(field, key)
        except:
            result = ""
        return result

    def set(self, filed, key, value):
        try:
            self.cf.set(field, key, value)
            cf.write(open(self.path, 'w'))
        except:
            return False
        return True
