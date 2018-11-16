#!/bin/bash
set -x
set -e

if [[ -z ${AutoTuneTMP_source_me_sourced} ]]; then
    source source-me.sh
fi

if [ ! -d "likwid/" ]; then
    git clone git@github.com:RRZE-HPC/likwid.git
    # mkdir likwid_install
    # cd likwid
    # likwid needs absolut paths, a bit more magic needed
    # sed -i -e 's/PREFIX = \/usr\/local#NO SPACE/PREFIX = likwid_install#NO SPACE/g' config.mk
    # sed -i -e 's/INSTALL_CHOWN = -g root -o root#NO SPACE/INSTALL_CHOWN = #NO SPACE/g' config.mk
    # cd ..
else
    git pull
fi

# cd likwid
# make -j${PARALLEL_BUILD}
# # make install
# cd ..
