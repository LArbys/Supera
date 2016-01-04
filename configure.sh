#!/bin/bash

# clean up previously set env
if [[ -z $FORCE_SUPERA_BASEDIR ]]; then
    # If SUPERA_BASEDIR not set, try to guess
    # Find the location of this script:
    me="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    # Find the directory one above.
    export SUPERA_BASEDIR="$( cd "$( dirname "$me" )" && pwd )"
    unset me;
else
    export SUPERA_BASEDIR=$FORCE_SUPERA_BASEDIR
fi

if [[ -z $SUPERA_BUILDDIR ]]; then
    export SUPERA_BUILDDIR=$SUPERA_BASEDIR/build
fi

export SUPERA_LIBDIR=$SUPERA_BUILDDIR/lib

# Abort if ROOT not installed. Let's check rootcint for this.
if [ `command -v rootcling` ]; then
    export SUPERA_ROOT6=1
else 
    if [[ -z `command -v rootcint` ]]; then
	echo
	echo Looks like you do not have ROOT installed.
	echo You cannot use LArLite w/o ROOT!
	echo Aborting.
	echo
	return 1;
    else
	export SUPERA_ROOT6=0
    fi
fi

if [[ -z $ROOTSYS ]]; then
    echo
    echo "****************** PyROOT WARNING ********************"
    echo "*                                                    *"
    echo "* Did not find your \$ROOTSYS. To use PyROOT feature, *"
    echo "* Make sure ROOT.py is installed (comes with ROOT).  *"
    echo "* You need to export \$PYTHONPATH to include the dir  *"
    echo "* where ROOT.py exists.                              *"
    echo "*                                                    *"
    echo "* Help to install PyROOT? See manual/contact author! *"
    echo "*                                                    *"
    echo "******************************************************"
    echo
fi

#
# Check LMDB
#
error=0
if [[ -z $LMDB_INCDIR ]]; then
    printf "\033[91merror\033[00m ... \$LMDB_INCDIR must be set for lmdb headers.\n";
    error=1;
fi
if [[ -z $LMDB_LIBDIR ]]; then
    printf "\033[91merror\033[00m ... \$LMDB_LIBDIR must be set for lmdb libraries.\n";
    error=1;
fi

if [[ -z $PROTOBUF_INCDIR ]]; then
    printf "\033[91merror\033[00m ... \$PROTOBUF_INCDIR must be set for protobuf headers.\n";
    error=1;
fi
if [[ -z $PROTOBUF_LIBDIR ]]; then
    printf "\033[91merror\033[00m ... \$PROTOBUF_LIBDIR must be set for protobuf libraries.\n";
    error=1;
fi

if [ $error -eq 1 ]; then
    case `uname -n` in
	(uboonegpvm*)
	printf "\033[95mrecovery\033[00m ... lucky you we can use local build by tmw...\n";
	export PROTOBUF_INCDIR=/uboone/app/users/tmw/projects/supera/protobuf/include
	export PROTOBUF_LIBDIR=/uboone/app/users/tmw/projects/supera/protobuf/lib
	export LMDB_INCDIR=/uboone/app/users/tmw/projects/supera/lmdb/libraries/liblmdb
	export LMDB_LIBDIR=/uboone/app/users/tmw/projects/supera/lmdb/libraries/liblmdb
	error=0;
	;;
	(*)
	unset SUPERA_BASEDIR;
	unset SUPERA_LIBDIR;
	unset SUPERA_BUILDDIR;
	unset SUPERA_ROOT6;
	return 1;
	;;
    esac
fi

echo
printf "\033[95mSUPERA_BASEDIR\033[00m  = $SUPERA_BASEDIR\n"
printf "\033[95mSUPERA_BUILDDIR\033[00m = $SUPERA_BUILDDIR\n"
printf "\033[95mSUPERA_LIBDIR\033[00m   = $SUPERA_LIBDIR\n"

mkdir -p $SUPERA_BUILDDIR;

export LD_LIBRARY_PATH=$SUPERA_LIBDIR:$LD_LIBRARY_PATH

if [ $LARLITE_OS -e 'Darwin' ]; then
    export DYLD_LIBRARY_PATH=$SUPERA_LIBDIR:$DYLD_LIBRARY_PATH
fi

echo
echo "Finish configuration. To build, type:"
echo "> cd \$SUPERA_BUILDDIR"
echo "> cmake \$SUPERA_BASEDIR"
echo "> make "
echo
