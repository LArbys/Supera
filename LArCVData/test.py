import ROOT
ROOT.gSystem.Load("libLArCV")
ROOT.gSystem.Load("libLArCVData")
from ROOT import larcv
c=larcv.IOManager("larcv::Image2D")
o=c(c.kWRITE)
o.reset()
o.set_verbosity(0)
o.set_out_file("aho.root")
o.initialize()
o.get_data("aho")
o.save_entry()
o.finalize()
