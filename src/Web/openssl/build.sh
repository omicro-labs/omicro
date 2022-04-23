#!/bin/sh

### make sure perl-IPC-Cmd is installed
sudo yum install perl-IPC-Cmd

### download openssl-3.0.2 and make install locally
mkdir -p install
wget --no-check-certificate https://www.openssl.org/source/openssl-3.0.2.tar.gz
tar zxf openssl-3.0.2.tar.gz
cdir=`pwd -P`
cd openssl-3.0.2
./config no-shared --prefix=$cdir/install
make
make install
