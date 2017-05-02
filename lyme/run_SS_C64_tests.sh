#!/bin/sh
#SBATCH --job-name=Anders_SS_C64
#
#SBATCH --mail-type=ALL
#SBATCH --mail-user=anders.maraviglia@gmail.com

srun --nodes=64 --ntasks=512 --overcommit -o experiment-13.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 1 &
srun --nodes=64 --ntasks=512 --overcommit -o experiment-14.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 2 &
srun --nodes=64 --ntasks=512 --overcommit -o experiment-15.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 4 &
srun --nodes=64 --ntasks=512 --overcommit -o experiment-16.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 8 &
wait
