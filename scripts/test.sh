GNU nano 7.2                                                                                    test_simp.sh                                                                                              
#!/bin/bash
#SBATCH --job-name=implementation_test
#SBATCH --output=out.out
#SBATCH --error=err.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=00:02:00
#SBATCH -p az4-n4090 -A pacc25
#SBATCH --cpus-per-task=16

date; hostname;


echo "compilation 2"
cd build
make -j4

echo " test "
./bin/murb-test

echo -n "Optim:  "
./bin/murb -n 2500 -i 200 --nv --im cpu+optim | grep FPS
echo -n "SIMD:   "
./bin/murb -n 2500 -i 200 --nv --im cpu+simd | grep FPS


export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=16
export OMP_SCHEDULE=static
./bin/murb -n 2500 -i 200 --nv --im cpu+omp+simd | grep FPS