#!/bin/bash
#SBATCH --job-name=murb_job
#SBATCH --output=murb_job_out_%j.out
#SBATCH --error=murb_job_err_%j.err
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
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -g" \
        -DCMAKE_CXX_FLAGS="-Wall -funroll-loops -march=native" \
        -DENABLE_MURB_CUDA=ON
make -j4
echo "✅ Compilation terminée"
echo ""

echo "=== exécution de la simulation ==="
./bin/murb -n 1000 -i 1000 -v --nv --im cpu+naive
# ./bin/murb -n 1000 -i 1000 -v --nv --im cpu+optim