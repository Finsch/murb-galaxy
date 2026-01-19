cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native"
make clean && make -j4
#./bin/murb -n 3000 -i 1000 --nv --im cpu+simd


# Différentes tailles de problème
for n in 500 1000 2000 3000 5000 8000 10000; do
    echo "=== Sans SIMD (n=$n) ==="
    ./bin/murb -n $n -i 1000 --nv --im cpu+optim | grep "Entire"
    echo "=== Avec SIMD (n=$n) ==="
    ./bin/murb -n $n -i 1000 --nv --im cpu+simd | grep "Entire"
    echo ""
done
