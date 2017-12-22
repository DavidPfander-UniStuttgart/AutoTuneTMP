#!/bin/bash
set -x
set -e

if [ ! -d "cppjit" ]; then
    git clone https://github.com/DavidPfander-UniStuttgart/cppjit.git
fi

cd cppjit
./build-all.sh $1
cd ..
