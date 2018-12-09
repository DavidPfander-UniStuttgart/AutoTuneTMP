#!/bin/bash
set -x
set -e

# dependencies first
./scripts/build-cppjit.sh $1

# need this, otherwise have to push arguments to called scripts
source source-me.sh

./scripts/build-likwid.sh
./scripts/build-boost.sh
./scripts/build-Vc.sh
./scripts/build-AutoTuneTMP.sh
