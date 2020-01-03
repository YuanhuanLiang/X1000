import re


def enum_f1(**enums):
    return type('Enum', (), enums)


def enum_f2(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    return type('Enum', (), enums)


def struct(*name):
    def decorator(func):
        def wrapper(*args, **kw):
            for i in range(len(name)):
                setattr(args[0], name[i], args[i+1])
            return func(*args, **kw)
        return wrapper
    return decorator


def str2int(snum):
    if snum.isdigit():
        return int(snum)
    base = re.match(r'^0[xX]', snum)
    if base:
        try:
            return int(snum[2:], 16)
        except ValueError:
            # print "ValueError: %s" % snum
            return -1
    return None


def is_one_empty(*args):
    for i in range(len(args)):
        if args[i] == '':
            return True
    return False


def human2bytes(sv):
    suffixes = {1024: ['KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB']}
    var = sv

    if not var or var.isdigit():
        return -1
    for k, v in suffixes.items():
        for i in range(0, len(v)):
            if var.find(v[i]) != -1:
                var = var.replace(v[i], "")
                if not var.isdigit():
                    return -1
                return int(var)*(k**(i+1))
    return -1


def bytes2human(n):
    suffixes = {1024: ['KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB']}
    prefix = {}
    for k, v in suffixes.items():
        for i, s in enumerate(suffixes[k]):
            prefix[s] = k**(i+1)
        for s in reversed(suffixes[k]):
            if n >= prefix[s]:
                value = float(n) / prefix[s]
                return '%d%s' % (value, s)
    return '%dB' % (n)
