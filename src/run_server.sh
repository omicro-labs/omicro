mkdir -p ../test/server/bin
mkdir -p ../test/server/conf
mkdir -p ../test/server/log

/bin/cp -f build/omicroserver ../test/server/bin
cd ../test/server

./setup_servers_local.sh
