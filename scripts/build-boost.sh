#!/bin/bash
set -x
set -e

if [[ -z ${AutoTuneTMP_source_me_sourced} ]]; then
    source source-me.sh
fi

if [ ! -d "boost" ]; then
    wget 'http://downloads.sourceforge.net/project/boost/boost/1.65.0/boost_1_65_0.tar.bz2'
    tar xf boost_1_65_0.tar.bz2
    mv boost_1_65_0 boost
    # # configure for gcc 7
    # if [[ "$MATRIX_MULTIPLICATION_TARGET" != "knl" ]]; then
    # 	echo "using gcc : 7.1 : /usr/bin/g++-7  ; " > boost/tools/build/src/user-config.jam
    # fi
fi

if [ ! -d "boost_install/" ]; then
    echo "building boost"
    cd boost
    ./bootstrap.sh --prefix="$BOOST_ROOT" -with-libraries=program_options,filesystem,system,test,thread,atomic,date_time
    ./b2 -j${PARALLEL_BUILD} variant=release install
    cd ..
fi


