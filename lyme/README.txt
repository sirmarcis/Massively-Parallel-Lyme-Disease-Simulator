Final Project: Parallel Simulation of Lyme Disease Spread Amongst Mice

Group Members:
Anthony Kim
Sara Khedr
Nick Fay
Anders Maraviglia

This program worked perfectly fine on AMOS and we were able to get all the results. 
This program takes in 2 arguments: Universe Size (integer 256, 512, 1024, 2048) and Number of Threads (integer 1, 2, 4, 8)

To Compile on Kratos:
--> make
To Run on Kratos:
--> mpirun --np [# MPI ranks] ./lyme --universeSize [size of the entire universe] --pthreads [number of threads per rank]


To Compile on AMOS BlueGene/Q:
--> module load xl
--> make -f Makefile_bgq

To Run on AMOS:
~~~~~~~~~~~~~~~~~~~~~~~~~~
Example snippet of script.sh file:
#!/bin/sh
srun --nodes=16 --ntasks=1024 --overcommit -o out_16_64_1.log /gpfs/u/barn/PCP6/PCP6kmnt/final/lyme.xl --universeSize 2048 --pthreads 1 &
srun --nodes=16 --ntasks=512 --overcommit -o out_16_32_2.log /gpfs/u/barn/PCP6/PCP6kmnt/final/lyme.xl --universeSize 2048 --pthreads 2 &
srun --nodes=16 --ntasks=256 --overcommit -o out_16_16_4.log /gpfs/u/barn/PCP6/PCP6kmnt/final/lyme.xl --universeSize 2048 --pthreads 4 &
srun --nodes=16 --ntasks=128 --overcommit -o out_16_8_8.log /gpfs/u/barn/PCP6/PCP6kmnt/final/lyme.xl --universeSize 2048 --pthreads 8 &
wait
~~~~~~~~~~~~~~~~~~~~~~~~~~

Example of a batch command on BGQ:
--> sbatch --partition medium -t 240 -n128 ./script.sh
