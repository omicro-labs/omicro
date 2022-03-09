
dr=`pwd -P`

cd $dr/g2log && mkdir build ; cd build && cmake .. && make
cd $dr/gmock-1.7.0 && ./configure && make

cd $dr/muduo
CC=gcc CXX=g++ BUILD_DIR=./build BUILD_TYPE=release BUILD_NO_EXAMPLES=1 . ./build.sh

cd $dr/gtest-1.7.0 && ./configure && make
