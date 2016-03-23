#
# This is an example GNUmakefile for my packages
#
PACKAGE_NAME  = LArCV

# specific names for this package
SHLIB = lib${PACKAGE_NAME}.so
SOURCES = $(wildcard *.cxx)
FMWK_HEADERS = LinkDef.h
HEADERS = $(filter-out $(FMWK_HEADERS), $(wildcard *.h))
IMPS = $(wildcard *.inl)

# include options for this package
INCFLAGS  = -I.                       #Include itself
INCFLAGS += -I$(OPENCV_INCDIR)

# platform-specific options
OSNAME          = $(shell uname -s)
HOST            = $(shell uname -n)
OSNAMEMODE      = $(OSNAME)

include $(LARCV_BASEDIR)/Makefile/Makefile.${OSNAME}

LDFLAGS += -L$(OPENCV_LIBDIR) -lopencv_core -lopencv_highgui -lopencv_imgcodecs
# call the common GNUmakefile
include $(LARCV_BASEDIR)/Makefile/GNUmakefile.CORE

