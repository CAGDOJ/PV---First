#!/bin/bash

docker run -it --rm -v $(pwd):/app pvfirst-env bash -c "
if [ ! -d build ]; then
    mkdir build
fi

cd build

if [ ! -f Makefile ]; then
    cmake ..
fi

make
./pvfirst
"