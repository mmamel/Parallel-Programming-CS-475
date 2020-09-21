#!/bin/bash
#SBATCH -J [job name]
#SBATCH -A cs475-575
#SBATCH -p class
#SBATCH --gres=gpu:1
#SBATCH -o [matrixmul.out]
#SBATCH -e [matrixmul.err]
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=mamel@oregonstate.edu
for t in 1 2 4 8
do
    /usr/local/apps/cuda/cuda-10.1/bin/nvcc -DNUMT=$t-o matrixmul matrixmul.cu 
    ./matrixmul
done

g++ -o printinfo printinfo.cpp usr/local/apps/cuda/cuda-10.1/lib64/libOpenCL.ao.1.1 -lm -fopen mp
./printinfo