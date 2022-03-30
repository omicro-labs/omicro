#!/bin/bash

if [[ -d ascii85 ]]; then
    /bin/rm -rf ascii85
fi

git clone https://github.com/dcurrie/ascii85.git
cp -f Makefile.omicro ascii85
cd ascii85
make -f Makefile.omicro
