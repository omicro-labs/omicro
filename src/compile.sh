#!/bin/bash

arg=$1

cd build
if [[ "$arg" = "force" ]]; then
	make clean
fi
./compile.sh

cd client/build
make clean; make
