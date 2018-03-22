#!/bin/bash
set -x
set -e

export BOOST_ROOT=/home/pfandedd/caro_boost_test

if [ ! -d "boost" ]; then
    wget 'http://downloads.sourceforge.net/project/boost/boost/1.62.0/boost_1_62_0.tar.bz2'
    tar xf boost_1_62_0.tar.bz2
    mv boost_1_62_0 boost
    # # configure for gcc 7
    # if [[ "$MATRIX_MULTIPLICATION_TARGET" != "knl" ]]; then
    # 	echo "using gcc : 7.1 : /usr/bin/g++-7  ; " > boost/tools/build/src/user-config.jam
    # fi
fi

if [ ! -d "boost_install/" ]; then
    echo "building boost"
    cd boost
    ./bootstrap.sh --prefix="$BOOST_ROOT"
    ./b2 -j${PARALLEL_BUILD} variant=release install
    cd ..
fi



