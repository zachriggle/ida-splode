import zlib
from glob import glob

def zipstreams(filename):
    """Return all zip streams and their positions in file."""
    with open(filename, 'rb') as fh:
        data = fh.read()
    i = 0
    while i < len(data):
        try:
            zo = zlib.decompressobj()
            yield i, zo.decompress(data[i:])
            i += len(data[i:]) - len(zo.unused_data)
        except zlib.error:
            i += 1

for filename in glob('*.mio'):
    print(filename)
    for i, data in zipstreams(filename):
        print (i, len(data))
import sys
zipstreams(sys.argv[-1])