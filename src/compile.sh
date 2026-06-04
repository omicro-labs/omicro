#!/bin/bash

arg=$1

cd build
if [[ "$arg" = "clean" ]]; then
	make clean
fi
./compile.sh

cd client/build
make clean; make
