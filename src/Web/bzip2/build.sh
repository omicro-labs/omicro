#!/bin/sh

mkdir install
pd=`pwd -P`

wget https://www.sourceware.org/pub/bzip2/bzip2-latest.tar.gz --no-check-certificate
tar zxf bzip2-latest.tar.gz

# cd to source or to the real version of latest
cd bzip2-1.0.8

make install PREFIX=$pd/install
