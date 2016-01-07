import os,sys
import google.protobuf as protobuf
import numpy as np
import lmdb as lmdb
import caffe_pb2 as caffe
from PIL import Image
import matplotlib.image

os.system("mkdir -p images")

def datum_to_array( datum ):
    h = datum.height
    w = datum.width
    image = np.zeros( (h,w), dtype=np.float )
    for t in xrange(0,h):
        image[t,:] = datum.float_data[ datum.width*(t): datum.width*(t+1)]
    return image

lmdb_dir = sys.argv[1]

lmdb_env = lmdb.open( lmdb_dir )
lmdb_txn = lmdb_env.begin()
lmdb_cursor = lmdb_txn.cursor()

datum = caffe.Datum()
for key, raw_datum in lmdb_cursor:
    datum.ParseFromString(raw_datum)
    label = datum.label
    data = datum_to_array( datum )
    rescaled = (255.0 / data.max() * (data - data.min())).astype(np.uint8)
    name = 'images/%s.png'%(key)
    print "Make ",name
    matplotlib.image.imsave(name, data)
    #scipy.misc.imsave(name, image)
    #im = Image.fromarray(rescaled)
    #im.save(name)
    
