
cd murb-se
rm -r build

# compile.
mkdir build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -g" -DCMAKE_CXX_FLAGS="-Wall -funroll-loops -march=native"
make -j4

# run the code. //ici
# ./bin/murb -n 1000 -i 1000 -v
./bin/murb -n 1000 -i 1000 -v --nv --im cpu+naive

# testing : validation du code.
./bin/murb-test


# chmod +x ./scripts/coderunner.sh 
# we are at murb-galaxy
# ./scripts/coderunner.sh 