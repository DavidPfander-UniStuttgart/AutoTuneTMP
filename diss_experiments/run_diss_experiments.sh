#!/bin/bash

cd ${AUTOTUNETMP_REPO_PATH}
./build-all.sh

./build/stream_thread_pool

cd ${ALL_CODE_ROOT_PATH}
cp ${AUTOTUNETMP_REPO_PATH}/stream_thread_pool*.csv results_raw/AutoTuneTMP/
