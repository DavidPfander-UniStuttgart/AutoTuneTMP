#!/bin/bash
set -x
set -e

if [ ! -d "cppjit" ]; then
    git clone git@github.com:DavidPfander-UniStuttgart/cppjit.git
fi

./cppjit/build-all.sh
