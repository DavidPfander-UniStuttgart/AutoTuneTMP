#!/bin/bash
set -x
set -e

if [[ -z ${AutoTuneTMP_source_me_sourced} ]]; then
    source source-me.sh
fi

if [ ! -d gcc-8.1.0 ] ; then
    wget ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-8.1.0/gcc-8.1.0.tar.xz
    tar xf gcc-8.1.0.tar.xz
fi

cd gcc-8.1.0
contrib/download_prerequisites
# export PREFIX=$GCC_ROOT

# # build binutils
# cd $src
# mkdir build-binutils
# cd build-binutils
# ../binutils-7.2.0/configure --prefix="$PREFIX" --disable-nls --disable-werror
# make -j ${PARALLEL_BUILD}
# make install

# cd src
# # If you wish to build these packages as part of GCC:
# mv libiconv-x.y.z gcc-x.y.z/libiconv # Mac OS X users
# mv gmp-x.y.z gcc-x.y.z/gmp
# mv mpfr-x.y.z gcc-x.y.z/mpfr
# mv mpc-x.y.z gcc-x.y.z/mpc
# cd ..

mkdir -p build-gcc
cd build-gcc
../configure --prefix="$GCC_ROOT_DIR" --disable-nls --disable-multilib --enable-languages=c,c++
make -j${PARALLEL_BUILD}
make install
cd ../..
