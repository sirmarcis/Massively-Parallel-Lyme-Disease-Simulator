#!/bin/sh
#SBATCH --job-name=Anders_SS_C16
#
#SBATCH --mail-type=ALL
#SBATCH --mail-user=anders.maraviglia@gmail.com

srun --nodes=16 --ntasks=128 --overcommit -o experiment-9.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 1 &
srun --nodes=16 --ntasks=128 --overcommit -o experiment-10.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 2 &
srun --nodes=16 --ntasks=128 --overcommit -o experiment-11.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 4 &
srun --nodes=16 --ntasks=128 --overcommit -o experiment-12.log /gpfs/u/home/PCP6/PCP6mrvg/scratch/lyme.xl --universeSize 2048 --pthreads 8 &
wait
