#!/bin/bash
cd murb-se/build

echo "Séquentiel:"
./bin/murb -n 1000 -i 1000 --nv --im cpu+optim 2>&1 | grep "FPS"

echo ""
echo "=== Exploration OpenMP ==="
for t in 1 2 4 8 12 14; do
    echo "   Threads=$t:"
    # export OMP_SCHEDULE="static,1"
    export OMP_SCHEDULE="dynamic,4"
    export OMP_NUM_THREADS=$t 
    ./bin/murb -n 1000 -i 1000 --nv --im cpu+omp 2>&1 | grep "FPS"
done


# chmod +x ./scripts/explore_omp.sh 
# we are at murb-galaxy
# ./scripts/explore_omp.sh 