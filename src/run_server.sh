mkdir -p ../test/server/bin
mkdir -p ../test/server/127.0.0.1/conf
mkdir -p ../test/server/log
/bin/cp -f build/omicroserver ../test/server/bin
cd ../test/server/bin

./omicroserver 127.0.0.1 12340

