#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

#!/bin/bash

CURR_DIR=$PWD
WAMR_DIR=${PWD}/../..
OUT_DIR=${PWD}/out

WASM_APPS=${PWD}/wasm-apps


rm -rf ${OUT_DIR}
mkdir ${OUT_DIR}
mkdir ${OUT_DIR}/wasm-apps


echo "#####################build basic project"
cd ${CURR_DIR}
mkdir -p cmake_build
cd cmake_build
cmake ..
make
if [ $? != 0 ];then
    echo "BUILD_FAIL basic exit as $?\n"
    exit 2
fi

cp -a basic ${OUT_DIR}

echo -e "\n"

echo "#####################build wasm apps"

cd ${WASM_APPS}

for i in `ls *.c`
do
APP_SRC="$i"
OUT_FILE=${i%.*}
OUT_FILE_WASM=${OUT_FILE}.wasm
OUT_FILE_WAT=${OUT_FILE}.wat

# use WAMR SDK to build out the .wasm binary
#--sysroot=${WAMR_DIR}/wamr-sdk/app/libc-builtin-sysroot  \
#        -Wl,--allow-undefined-file=${WAMR_DIR}/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt \

#--sysroot=/opt/wasi-sdk/share/wasi-sysroot \
/opt/wasi-sdk/bin/clang     \
        --target=wasm32 -O0 -z stack-size=4096 -Wl,--initial-memory=65536 \
        --sysroot=${WAMR_DIR}/wamr-sdk/app/libc-builtin-sysroot  \
        -Wl,--allow-undefined-file=${WAMR_DIR}/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt \
        -Wl,--strip-all,--no-entry -nostdlib \
        -Wl,--export=generate_float \
        -Wl,--export=float_to_string \
        -Wl,--export=calculate\
        -Wl,--export=malloc \
        -Wl,--export=free \
        -Wl,--allow-undefined \
        -o ${OUT_DIR}/wasm-apps/${OUT_FILE_WASM} ${APP_SRC}

wasm2wat ${OUT_DIR}/wasm-apps/${OUT_FILE_WASM} -o ${OUT_DIR}/wasm-apps/${OUT_FILE_WAT}

if [ -f ${OUT_DIR}/wasm-apps/${OUT_FILE_WASM} ]; then
        echo "build ${OUT_FILE_WASM} success"
else
        echo "build ${OUT_FILE_WASM} fail"
fi
done
echo "####################build wasm apps done"
