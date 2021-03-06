#!/bin/bash
# set -e
# set -x

# it is possible for the configuration to have been done by a project that uses AutoTuneTMP
if [[ -z "$CONFIGURATION_DONE" ]]; then
    if [[ -z "$1" || "$1" != "circle" ]]; then
        #use all available CPUs
        export PARALLEL_BUILD=$((`lscpu -p=cpu | wc -l`-4))
    else
        # circle uses special parallelism settings in the build scripts to maximize performance per scripts
        # all threads ins't always possible, because of memory limititations (4G)
        export PARALLEL_BUILD=2
    fi
    echo "parallel build (-j for make): $PARALLEL_BUILD"

    export CC=gcc
    export CXX=g++
    export CXX_FLAGS="-Wall -Wextra -march=native -mtune=native"
    export CONFIGURATION_DONE=true
else
    echo "AutoTuneTMP: configuration already done, skipping..."
fi

if [ ! -d "cppjit" ]; then
    git clone git@github.com:DavidPfander-UniStuttgart/cppjit.git
fi

# source the remaining dependencies recursively
source $(readlink -f $(dirname "$BASH_SOURCE"))/cppjit/source-me.sh

REL_BASE_PATH=`dirname "$BASH_SOURCE"`
BASE_PATH=`readlink -f $REL_BASE_PATH`

# note that GCC_ROOT is already used by gcc itself, don't set!
export GCC_ROOT_DIR=$BASE_PATH/gcc_install
export Vc_ROOT=$BASE_PATH/Vc_install
export BOOST_ROOT=$BASE_PATH/boost_install
export VC_ROOT=$BASE_PATH/Vc_install
# export CPPJIT_ROOT=$BASE_PATH/cppjit_install
export AUTOTUNETMP_ROOT=$BASE_PATH/AutoTuneTMP_install
export LD_LIBRARY_PATH=$BASE_PATH/boost_install/lib:$BASE_PATH/Vc_install/lib:$BASE_PATH/libkwid:$BASE_PATH:$BASE_PATH/likwid/ext/hwloc:$BASE_PATH/likwid/ext/lua:$LD_LIBRARY_PATH
export AutoTuneTMP_source_me_sourced=1
