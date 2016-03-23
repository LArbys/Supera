#
# This is an example GNUmakefile for my packages
#
PACKAGE_NAME  = APICaffe

# specific names for this package
SHLIB = lib${PACKAGE_NAME}.so
SOURCES = $(wildcard *.cxx)
FMWK_HEADERS = LinkDef.h db.h caffe.pb.h db_lmdb.h
HEADERS = $(filter-out $(FMWK_HEADERS), $(wildcard *.h))

# include options for this package
INCFLAGS  = -I.                       #Include itself
INCFLAGS += -I$(LARCV_BASEDIR)
#INCFLAGS += -I$(LMDB_INCDIR)
#INCFLAGS += -I$(PROTOBUF_INCDIR)
INCFLAGS += -I$(OPENCV_INCDIR)

# platform-specific options
OSNAME          = $(shell uname -s)
HOST            = $(shell uname -n)
OSNAMEMODE      = $(OSNAME)

include $(LARCV_BASEDIR)/Makefile/Makefile.${OSNAME}

LDFLAGS += -L$(LARCV_LIBDIR) -lLArCV
LDFLAGS += -L$(LMDB_LIBDIR) -llmdb
LDFLAGS += -L$(PROTOBUF_LIBDIR) -lprotobuf
LDFLAGS += -L$(OPENCV_LIBDIR) -lopencv_core -lopencv_highgui -lopencv_imgcodecs
# call the common GNUmakefile
include $(LARCV_BASEDIR)/Makefile/GNUmakefile.CORE

