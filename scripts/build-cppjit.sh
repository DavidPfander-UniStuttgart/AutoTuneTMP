#!/bin/bash
set -x
set -e

if [ ! -d "cppjit" ]; then
    git clone git@github.com:DavidPfander-UniStuttgart/cppjit.git
fi

cd cppjit
./build-all.sh $1
cd ..
