#!/bin/bash

# clean up previously set env
if [[ -z $FORCE_LARCAFFE_BASEDIR ]]; then
    # If LARCAFFE_BASEDIR not set, try to guess
    # Find the location of this script:
    LARCAFFE_BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    # Find the directory one above.
    #export LARCAFFE_BASEDIR="$( cd "$( dirname "$me" )" && pwd )"
    #unset me;
else
    export LARCAFFE_BASEDIR=$FORCE_LARCAFFE_BASEDIR
fi

if [[ -z $LARCAFFE_BUILDDIR ]]; then
    export LARCAFFE_BUILDDIR=$LARCAFFE_BASEDIR/build
fi

export LARCAFFE_LIBDIR=$LARCAFFE_BUILDDIR/lib
export LARCAFFE_INCDIR=$LARCAFFE_BASEDIR

# Abort if ROOT not installed. Let's check rootcint for this.
if [ `command -v rootcling` ]; then
    export LARCAFFE_ROOT6=1
else 
    if [[ -z `command -v rootcint` ]]; then
	echo
	echo Looks like you do not have ROOT installed.
	echo You cannot use LArLite w/o ROOT!
	echo Aborting.
	echo
	return 1;
    else
	export LARCAFFE_ROOT6=0
    fi
fi

# Check LMDB
error=0
if [[ -z $LMDB_INCDIR ]]; then
    printf "\033[95mwarning\033[00m ... \$LMDB_INCDIR must be set for lmdb headers.\n";
    error=1;
fi
if [[ -z $LMDB_LIBDIR ]]; then
    printf "\033[95mwarning\033[00m ... \$LMDB_LIBDIR must be set for lmdb libraries.\n";
    error=1;
fi

# Check protobuf
if [[ -z $PROTOBUF_INCDIR ]]; then
    printf "\033[95mwarning\033[00m ... \$PROTOBUF_INCDIR must be set for protobuf headers.\n";
    error=1;
fi
if [[ -z $PROTOBUF_LIBDIR ]]; then
    printf "\033[95mwarning\033[00m ... \$PROTOBUF_LIBDIR must be set for protobuf libraries.\n";
    error=1;
fi
if [[ -z `command -v protoc` ]]; then
    printf "\033[95mwarning\033[00m ... protoc (protobuf compiler) is not available. (needed to generate protobuf header.\n";
    error=1;
fi

if [ $error -eq 1 ]; then
    case `uname -n` in
	(uboonegpvm*)
	printf "\033[93mrecovery\033[00m ... lucky you we can use local build by tmw...\n";
	export PROTOBUF_INCDIR=/uboone/app/users/tmw/projects/supera/protobuf/include
	export PROTOBUF_LIBDIR=/uboone/app/users/tmw/projects/supera/protobuf/lib
	export LMDB_INCDIR=/uboone/app/users/tmw/projects/supera/lmdb/libraries/liblmdb
	export LMDB_LIBDIR=/uboone/app/users/tmw/projects/supera/lmdb/libraries/liblmdb
	export PATH=$PATH:/uboone/app/users/tmw/projects/supera/protobuf/bin
	error=0;
	;;
	(*)
	printf "\033[91merror\033[00m ... aborting configuration.\n";
	unset LARCAFFE_BASEDIR;
	unset LARCAFFE_LIBDIR;
	unset LARCAFFE_BUILDDIR;
	unset LARCAFFE_ROOT6;
	unset LARCAFFE_INCDIR;
	return 1;
	;;
    esac
fi

echo
printf "\033[95mLARCAFFE_BASEDIR\033[00m  = $LARCAFFE_BASEDIR\n"
printf "\033[95mLARCAFFE_BUILDDIR\033[00m = $LARCAFFE_BUILDDIR\n"
printf "\033[95mLARCAFFE_LIBDIR\033[00m   = $LARCAFFE_LIBDIR\n"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PROTOBUF_LIBDIR:$LMDB_LIBDIR:$LARCAFFE_LIBDIR;
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$PROTOBUF_LIBDIR:$LMDB_LIBDIR:$LARCAFFE_LIBDIR;

mkdir -p $LARCAFFE_BUILDDIR;

if [ ! -f $LARCAFFE_BASEDIR/SuperaCore/caffe.pb.h ]; then
    printf "\033[93mnotice\033[00m ... generating caffe proto-buf source code (one-time operation)\n"
    protoc $LARCAFFE_BASEDIR/SuperaCore/caffe.proto --proto_path=$LARCAFFE_BASEDIR/SuperaCore --cpp_out=$LARCAFFE_BASEDIR/SuperaCore/
    mv $LARCAFFE_BASEDIR/SuperaCore/caffe.pb.cc $LARCAFFE_BASEDIR/SuperaCore/caffe.pb.cxx
fi

export LD_LIBRARY_PATH=$LARCAFFE_LIBDIR:$LD_LIBRARY_PATH

if [ $LARLITE_OS -e 'Darwin' ]; then
    export DYLD_LIBRARY_PATH=$LARCAFFE_LIBDIR:$DYLD_LIBRARY_PATH
fi

if [ -d $MRB_TOP/srcs/uboonecode/uboone ]; then
    echo Found local uboonecode @ \$MRB_TOP=${MRB_TOP}
    if [ ! -d $MRB_TOP/srcs/uboonecode/uboone/Supera ]; then
	echo Making a sym-link for LArSoft API...
	ln -s $LARCAFFE_BASEDIR/APILArSoft $MRB_TOP/srcs/uboonecode/uboone/Supera
    fi
fi

LARCAFFE_CXX=clang
if [ -z `command -v $LARCAFFE_CXX` ]; then
    LARCAFFE_CXX=g++
    if [ -z `command -v $LARCAFFE_CXX` ]; then
        echo
        echo Looks like you do not have neither clang or g++!
        echo You need one of those to compile LArCaffe... Abort config...
        echo
        return 1;
    fi
fi

echo
echo "Finish configuration. To build, type:"
echo "> cd \$LARCAFFE_BUILDDIR"
echo "> cmake \$LARCAFFE_BASEDIR -DCMAKE_CXX_COMPILER=$LARCAFFE_CXX"
echo "> make "
echo
