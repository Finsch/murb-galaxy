#!/bin/bash

echo "=== Menu Build & Run ==="
echo "1. Valider le code"
echo "2. Exécuter la simulation"
read -p "Choix (1 ou 2): " choice

cd murb-se || { echo "Erreur: dossier murb-se introuvable"; exit 1; }

case $choice in
    1)
        # compile.
        echo "=== Compilation ==="
        rm -rf build
        mkdir build
        cd build
        cmake .. -G"Unix Makefiles" -DCMAKE_CXX_COMPILER=g++ \
                -DCMAKE_BUILD_TYPE=RelWithDebInfo \
                -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -g" \
                -DCMAKE_CXX_FLAGS="-Wall -funroll-loops -march=native"
        make -j4
        echo "✅ Compilation terminée"
        echo ""

        # testing : valider le code
        echo "=== testing : validation du code ==="
        ./bin/murb-test
        ;;
    2)
        echo "=== exécution de la simulation ==="
        # run the code. //ici
        cd build
        # ./bin/murb -n 1000 -i 1000 -v
        ./bin/murb -n 1000 -i 1000 -v --nv --im cpu+naive
        ;;
    *)
        echo "Choix invalide. Exit."
        exit 1
        ;;
esac


# chmod +x ./scripts/coderunner.sh 
# we are at murb-galaxy
# ./scripts/coderunner.sh 