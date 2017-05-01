#!/bin/sh
#SBATCH --job-name=Anders_SS_C4
#
#SBATCH --mail-type=ALL
#SBATCH --mail-user=anders.maraviglia@gmail.com

srun --nodes=4 --ntasks=32 --overcommit -o experiment-5.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 1 &
srun --nodes=4 --ntasks=32 --overcommit -o experiment-6.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 2 &
srun --nodes=4 --ntasks=32 --overcommit -o experiment-7.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 4 &
srun --nodes=4 --ntasks=32 --overcommit -o experiment-8.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 8 &
wait
