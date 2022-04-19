#!/bin/bash

arg=$1

cd build
if [[ "$arg" = "f" ]]; then
	make clean
fi
./compile.sh

cd client/build
make clean; make
