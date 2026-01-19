#!/bin/bash
n=2000
iter=100
threads=8

echo "Scheduler,ChunkSize,FPS,Speedup_vs_static1"
base_fps=1876.5  # Ton résultat avec static,1

# Test différents schedulers
for scheduler in "static" "dynamic" "guided"; do
    for chunk in 1 2 4 8 16 32 64; do
        if [ "$scheduler" = "guided" ] && [ $chunk -ne 1 ]; then
            continue  # guided n'a pas de chunk size
        fi
        
        if [ "$scheduler" = "guided" ]; then
            export OMP_SCHEDULE="guided"
        else
            export OMP_SCHEDULE="$scheduler,$chunk"
        fi
        
        fps=$(OMP_NUM_THREADS=$threads ./bin/murb -n $n -i $iter --im cpu+omp --nv 2>/dev/null | grep "FPS" | awk '{print $3}')
        speedup=$(echo "scale=2; $fps / $base_fps" | bc)
        echo "$scheduler,$chunk,$fps,$speedup"
    done
done