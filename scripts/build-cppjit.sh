#!/bin/bash
set -x
set -e

if [ -z ${AutoTuneTMP_source_me_sourced} ] ; then
    source source-me.sh
else
    echo "AutoTuneTMP: source-me.sh already sourced"
fi

if [ ! -d "cppjit" ]; then
    git clone git@github.com:DavidPfander-UniStuttgart/cppjit.git
fi

./cppjit/build-all.sh
