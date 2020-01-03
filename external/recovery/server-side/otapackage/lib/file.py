import os

def is_writeable(path, check_parent=False):
    if os.access(path, os.F_OK) and os.access(path, os.W_OK):
        return True

    if os.access(path, os.F_OK) and not os.access(path, os.W_OK):
        return False

    if check_parent is False:
        return False

    parent_dir = os.path.dirname(path)
    if not os.access(parent_dir, os.F_OK):
        return False
    return os.access(parent_dir, os.W_OK)


def is_readable(path):
    if os.access(path, os.F_OK) and os.access(path, os.R_OK):
        return True
    return False


def get_size(path):
    return os.path.getsize(path)


def split(fromfile, todir, chunksize, func):
    if not os.path.exists(todir):
        os.makedirs(todir)
    # else:
    #     for fname in os.listdir(todir):
    #         os.remove(os.path.join(todir, fname))
    partnum = 1
    f = open(fromfile, 'rb')
    while 1:
        chunk = f.read(chunksize)
        if not chunk:
            break
        filename = os.path.join(
            todir, ('%s_%03d' % (os.path.basename(fromfile), partnum)))
        fileobj = open(filename, 'wb')
        fileobj.write(chunk)
        fileobj.close()
        if func != None:
            func(filename)
        partnum += 1
    f.close()
    return (partnum-1)
