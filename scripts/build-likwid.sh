#!/bin/bash
set -x
set -e

if [[ -z ${AutoTuneTMP_source_me_sourced} ]]; then
    source source-me.sh
fi

if [ ! -d "likwid/" ]; then
    git clone git@github.com:RRZE-HPC/likwid.git
    cd likwid/
    git checkout 4.3.4
    cd ..
    # mkdir likwid_install
    # cd likwid
    # likwid needs absolut paths, a bit more magic needed
    # sed -i -e 's/PREFIX = \/usr\/local#NO SPACE/PREFIX = likwid_install#NO SPACE/g' config.mk
    # sed -i -e 's/INSTALL_CHOWN = -g root -o root#NO SPACE/INSTALL_CHOWN = #NO SPACE/g' config.mk
    # cd ..
# else
#     git pull
fi

cd likwid
make -j${PARALLEL_BUILD}
# make install
cd ..

# ensure necessary symlinks exist
ln -sf likwid/ext/hwloc/liblikwid-hwloc.so liblikwid-hwloc.so.4.3
ln -sf likwid/ext/lua/liblikwid-lua.so liblikwid-lua.so.4.3
