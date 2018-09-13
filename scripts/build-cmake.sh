#!/bin/bash
set -x
set -e

if [ -z ${matrix_multiplication_source_me_sourced} ] ; then
    source source-me.sh
fi

if [ ! -d cmake ] ; then
    wget https://cmake.org/files/v3.12/cmake-3.12.2-Linux-x86_64.tar.gz
    tar xf cmake-3.12.2-Linux-x86_64.tar.gz
    mv cmake-3.12.2-Linux-x86_64 cmake
fi
