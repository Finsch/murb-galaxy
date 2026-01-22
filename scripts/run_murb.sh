#!/bin/bash
#SBATCH --job-name=murb_job
#SBATCH --output=job_out_%j.out
#SBATCH --error=job_err_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:02:00
#SBATCH --partition=az4-mixed
#SBATCH -A pacc25

date
hostname


cd ~/porkypig/murb-se

# compile.
echo "=== Compilation ==="
rm -rf build
mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG"
make -j4
echo "✅ Compilation terminée"
echo ""

echo "=== exécution de la simulation ==="
# ./bin/murb -n 1000 -i 1000 -v --nv --im cpu+naive
# ./bin/murb -n 1000 -i 1000 -v --nv --im cpu+optim
# export OMP_NUM_THREADS=8
# export OMP_SCHEDULE="static,1"
# export OMP_SCHEDULE="dynamic,4"
# ./bin/murb -n 1000 -i 1000 -v --nv --im cpu+omp

echo "Séquentiel:"
./bin/murb -n 500 -i 50 --nv --im cpu+optim 2>&1 | grep "FPS"

echo ""
echo "=== Exploration OpenMP ==="
for t in 1 2 4 8 12 14; do
    echo "   Threads=$t:"
#     export OMP_SCHEDULE="static,1"
    export OMP_SCHEDULE="dynamic,4"
    export OMP_NUM_THREADS=$t 
    ./bin/murb -n 500 -i 50 --nv --im cpu+omp 2>&1 | grep "FPS"
done
