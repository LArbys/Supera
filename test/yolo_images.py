import os,sys
import numpy as np
from PIL import Image # 'pip install Image' to get this one
import matplotlib.image
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import ROOT as rt

def vector_to_array( planeid, tree ):
    #nwires = tree.wires[planeid]
    #nticks = tree.nticks
    nwires = 448
    nticks = 448
    image = np.zeros( (nwires,nticks), dtype=np.int )
    print tree.img_plane2.size()
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

ttree = rt.TChain( "yolo/imgtree" )
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
        label = "%d_%d_%d_plane%d_mode%d" % ( ttree.run, ttree.subrun, ttree.event, planeid, ttree.mode )
        name = '%s/plane%d/%s.png'%(outdir,planeid,label)

        # RGB using unscaled ADC values
        print name

        imgplot = plt.imshow(img)
        currentAxis = plt.gca()
        boxes = []
        branch = None
        if planeid==0:
            branch = ttree.LoLeft_w_plane0
        elif planeid==1:
            branch = ttree.LoLeft_w_plane1
        elif planeid==2:
            branch = ttree.LoLeft_w_plane2

        for bb in range(0,branch.size()):
            if planeid==2:
                x1 = ttree.LoLeft_w_plane2.at(bb)
                y1 = ttree.LoLeft_t_plane2.at(bb)
                x2 = ttree.HiRight_w_plane2.at(bb)
                y2 = ttree.HiRight_t_plane2.at(bb)
            elif planeid==1:
                x1 = ttree.LoLeft_w_plane1.at(bb)
                y1 = ttree.LoLeft_t_plane1.at(bb)
                x2 = ttree.HiRight_w_plane1.at(bb)
                y2 = ttree.HiRight_t_plane1.at(bb)
            elif planeid==0:
                x1 = ttree.LoLeft_w_plane0.at(bb)
                y1 = ttree.LoLeft_t_plane0.at(bb)
                x2 = ttree.HiRight_w_plane0.at(bb)
                y2 = ttree.HiRight_t_plane0.at(bb)
                
            print (x1,y1),(x2,y2),"(dx,dy)=",(x2-x1,y2-y1)
            bbox = Rectangle((x1,y1),x2-x1,y2-y1,alpha=1,facecolor='None',edgecolor='Red')
            currentAxis.add_patch(bbox)
            boxes.append(bbox)
        plt.axis('off')
        imgplot.axes.get_xaxis().set_visible(False)
        imgplot.axes.get_yaxis().set_visible(False)
        if len(boxes)>0:
            plt.savefig(name,bbox_inches='tight',pad_inches=0)
        for b in boxes:
            b.remove()
        #matplotlib.image.imsave(name,img)
        # greyscale using scaled ADC values
        #im = Image.fromarray(rescaled)
        #im.save(name)
    entry += 1
    bytes = ttree.GetEntry( entry )
    
