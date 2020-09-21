main.cpp should be run on the DGX server by running the command 
$ sbatch run 
the run script will compile and execute, the performance will be 
outputed to Project7B.out 
the sine wave will be outputed to a file called output.csv 

**Note** the 4-thread openmp performance should be run on either the flip or rabbit servers using command
$ g++ openmp.cpp -lm -fopenmp
$ a.out