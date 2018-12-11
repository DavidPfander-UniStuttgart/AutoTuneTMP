#!/bin/bash
set -x
set -e

if [[ -z ${AutoTuneTMP_source_me_sourced} ]]; then
    source source-me.sh
fi

git pull


mkdir -p build
cd build

echo "compiling AutoTuneTMP"
# detection of Vc doesn't work with a relative path
# > cmake_AutoTuneTMP.log 2>&1
cmake -DBOOST_ROOT=$BOOST_ROOT -DCMAKE_INSTALL_PREFIX="$AUTOTUNETMP_ROOT" -DCPPJIT_ROOT="$CPPJIT_ROOT" -DVc_ROOT="$Vc_ROOT" -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CXX_FLAGS=$CXX_FLAGS -DCMAKE_BUILD_TYPE=release -DWITH_LIKWID=ON ../

# uses more than 4G with 4 threads (4G limit on Circle CI)
#   > make_AutoTuneTMP.log 2>&1
make -j${PARALLEL_BUILD} VERBOSE=1
make VERBOSE=1 install
cd ../..
