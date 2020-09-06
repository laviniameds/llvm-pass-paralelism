#!/bin/bash

# build project
cd build/ && cmake .. && make && cd .. && echo ""\

# testing pass
src=$1
obj=${1%.c}.ll
opt=${1%.c}.opt.ll

clang-10 -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${obj}

opt-10 -S -mem2reg ${obj} > ${opt}

opt-10 -load build/parallelism/libParallelismPass.so -parallelism < ${opt} > /dev/null
