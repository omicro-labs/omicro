OLD_PWD="$( pwd )"

export BOOST_INC_PATH=/usr/local/include/boost
export BOOST_LIB_PATH=/usr/local/lib


echo "" && echo "" && echo "[-------------------------------]" && echo "   essential" && echo "[-------------------------------]" && \
    cd ./essential/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   server_lib" && echo "[-------------------------------]" && \
    cd ../server_lib/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   server" && echo "[-------------------------------]" && \
    cd ../server/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   client_lib" && echo "[-------------------------------]" && \
    cd ../client_lib/ && make && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   client_with_asio" && echo "[-------------------------------]" && \
    cd ../client_with_asio/ && make && \
echo ""


# restore old path.
cd $OLD_PWD
