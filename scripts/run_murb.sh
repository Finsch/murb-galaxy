#!/bin/bash
#SBATCH --job-name=murb_job
#SBATCH --output=job_out_%j.out
#SBATCH --error=job_err_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=00:02:00
#SBATCH --partition=az4-mixed
#SBATCH -A pacc25
#SBATCH --cpus-per-task=16

date
hostname

cd ~/porkypig/murb-se
# cd ~/porkypig/murb-mo

# compile.
echo "=== Compilation ==="
rm -rf build
mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG" \
        -DENABLE_MURB_CUDA=ON
make -j4
echo "✅ Compilation terminée"
echo ""

echo "=== exécution de la simulation ==="
echo "   gpu+optim:"
./bin/murb -n 1000 -i 1000 -v --nv --im gpu+optim 2>&1 | grep "FPS"

echo "=== validation par tests ==="
./bin/murb-test