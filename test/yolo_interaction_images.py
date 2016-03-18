import os,sys
import numpy as np
from PIL import Image # 'pip install Image' to get this one
import matplotlib.image
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import ROOT as rt

def vector_to_array( planeid, tree ):
    #nwires = tree.nwires
    #nticks = tree.nticks
    nwires = 224
    nticks = 224
    image = np.zeros( (nwires,nticks), dtype=np.int )
    for w in range(0,nwires):
        for t in range(0,nticks):
            if planeid==0:
                image[t,w] = tree.img_plane0.at( w*nticks + t )
            elif planeid==1:
                image[t,w] = tree.img_plane1.at( w*nticks + t )
            elif planeid==2:
                image[t,w] = tree.img_plane2.at( w*nticks + t )
    return image

if len(sys.argv)!=3:
    print "usage: python make_pngs.py [lmdb folder/db] [output dir]"
    print "Will dump out a bunch of RGB pngs whose name will be the key."
    sys.exit(-1)

rootfiles = sys.argv[1]
outdir    = sys.argv[2]

os.system("mkdir -p %s"%(outdir))
os.system("mkdir -p %s/plane0"%(outdir))
os.system("mkdir -p %s/plane1"%(outdir))
os.system("mkdir -p %s/plane2"%(outdir))

ttree = rt.TChain( "yolo/bbtree" )
ttree.Add( rootfiles )

entry = 0
bytes = ttree.GetEntry( entry )

while bytes!=0:
    
    #if ttree.event!=32485:
    #    entry += 1
    #    bytes = ttree.GetEntry( entry )
    #    continue

    for planeid in range(0,3):
        img = vector_to_array( planeid, ttree )
        strlabel = ""
        for s in ttree.label:
            if s=='\0':
                break
            else:
                strlabel += s
        label = "%s_%d_%d_%d_%d" % ( strlabel, ttree.run, ttree.subrun, ttree.event, ttree.ibox )
        imgname = "%s/plane%d/%s.png"%(outdir,planeid,label)
        
        # RGB using unscaled ADC values
        print imgname

        matplotlib.image.imsave(imgname,img)
        #imgplot = plt.imshow(img)
        #currentAxis = plt.gca()
        #plt.axis('off')
        #imgplot.axes.get_xaxis().set_visible(False)
        #imgplot.axes.get_yaxis().set_visible(False)
        #plt.savefig(name,bbox_inches='tight',pad_inches=0)
    entry += 1
    bytes = ttree.GetEntry( entry )
    
