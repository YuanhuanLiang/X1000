import logging
from otapackage import config
from logging.handlers import RotatingFileHandler

class Logger:
    prompt = config.log_prompt
    logger_file = None
    logger_console = None

    logger_types = ("file", "console")
    logger = {logger_types[0]: logger_file,
              logger_types[1]: logger_console}

    levels = {"n": logging.NOTSET,
              "d": logging.DEBUG,
              "i": logging.INFO,
              "w": logging.WARN,
              "e": logging.ERROR,
              "c": logging.CRITICAL}
    # The default system settting print level is "warning", 
    # even though do not set it manually
    logger_level_default = {logger_types[0]: levels['w'],
                            logger_types[1]: levels['w']}

    log_file = config.log_logname
    log_max_byte = config.log_size
    log_backup_count = 5

    @classmethod
    def get_logger(cls, logname):

        if not logname:
            return None

        if logname not in cls.logger_types:
            return None

        if cls.logger[logname] is not None:
            return cls.logger[logname]

        log_fmt = logging.Formatter(
            '''%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s: %(message)s''')
        if logname == cls.logger_types[0]:

            cls.logger[logname] = logging.Logger(
                "%s-%s" % (cls.prompt, logname))

            handler = RotatingFileHandler(
                filename=cls.log_file, maxBytes=cls.log_max_byte,
                backupCount=cls.log_backup_count)

        elif logname == cls.logger_types[1]:
            cls.logger[logname] = logging.getLogger(
                "%s-%s" % (cls.prompt, logname))

            handler = logging.StreamHandler()

        handler.setFormatter(log_fmt)
        cls.logger[logname].addHandler(handler)
        # cls.logger[logname].setLevel(cls.logger_level_default[logname])
        return cls.logger[logname]

    @classmethod
    def set_level(cls, logname, levelname):
        cls.get_logger(logname).setLevel(cls.levels[levelname])
