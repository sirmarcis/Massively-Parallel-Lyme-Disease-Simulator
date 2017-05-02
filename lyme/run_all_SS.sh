#!/bin/sh

sbatch --partition small -t 240 -N4 ./run_SS_C1_tests.sh
sbatch --partition small -t 240 -N16 ./run_SS_C4_tests.sh
sbatch --partition small -t 240 -N64 ./run_SS_C16_tests.sh
sbatch --partition medium -t 240 -N256 ./run_SS_C64_tests.sh
