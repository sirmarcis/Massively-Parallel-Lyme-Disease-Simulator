#!/bin/sh
#SBATCH --job-name=Anders_SS_C1
#
#SBATCH --mail-type=ALL
#SBATCH --mail-user=anders.maraviglia@gmail.com

srun --nodes=1 --ntasks=8 --overcommit -o experiment-1.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 1 &
srun --nodes=1 --ntasks=8 --overcommit -o experiment-2.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 2 &
srun --nodes=1 --ntasks=8 --overcommit -o experiment-3.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 4 &
srun --nodes=1 --ntasks=8 --overcommit -o experiment-4.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 8 &
wait
