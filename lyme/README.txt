README for lyme project c code:
By: a bunch of scrubs

To compile:
	'$ make'
	(BGQ need to run: '$ module load xl')
To run:
	'$ mpirun --np [# MPI ranks] ./lyme --pthreads [number of threads per rank] --universeSize [size of the entire universe]'

To run a batch on BGQ:
	'$ sbatch --partition small -t 240 -N4 ./run_anders_tests.sh'

To run anders batch (2048, 16 Compute nodes):
	'$ sbatch --partition small -t 240 -N64 ./run_strong_scaling_2048.sh'

Scaling Tests (run each set of tests with only ONE compute node):

Nick: 4096x4096
Sara: 2048x2048
Anders: 1024x1024
Anthony: 256x256

awefnlawef;aewf'anwe
awe
pnaw
evn
awenv
awev
[awev

awevb
aweb

web
aweb
aweb
[aweb
aweb
aweb
