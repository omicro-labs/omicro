git clone https://github.com/pq-crystals/dilithium.git
cd dilithium/ref
cmake ..
make

echo "Rename some CTYPTO SIZE to *_DL names due to #define conflicts"
echo "dilithium/ref/ref/ has tests and library .a files"

