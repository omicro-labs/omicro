PD="$(pwd)"

export BOOST_INC_PATH=/usr/local/include/boost
export BOOST_LIB_PATH=/usr/local/lib

`rm -f client_with_asio/client_with_asio 2>/dev/null ;\
    rm -f server/server 2>/dev/null ;\
    rm -f server_lib/asio_kcp_server.a 2>/dev/null;\
    rm -f asio_kcp_utest/asio_kcp_utest 2>/dev/null;\
    rm -f asio_kcp_client_utest/asio_kcp_client_utest 2>/dev/null;\
`

echo "" && echo "" && echo "[-------------------------------]" && echo "   essential" && echo "[-------------------------------]" && \
    cd $PD/essential/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   client_lib" && echo "[-------------------------------]" && \
    cd $PD/client_lib/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   server_lib" && echo "[-------------------------------]" && \
    cd $PD/server_lib/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   server" && echo "[-------------------------------]" && \
    cd $PD/server/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   client_with_asio" && echo "[-------------------------------]" && \
    cd $PD/client_with_asio/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   asio_kcp_utest" && echo "[-------------------------------]" && \
    cd $PD/asio_kcp_utest/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   kcp_client_utest" && echo "[-------------------------------]" && \
    cd $PD/asio_kcp_client_utest/ && make && \
echo ""


#cd $PD/server_lib
#make
#cd $PD/client_lib
#make
