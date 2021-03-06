# Parallel Simulation of Lyme Disease Spread Amongst Mice

A massively parallel disease modeler for Blue Gene /Q


For our project, we will be simulating Lyme Disease using ROSS. ROSS will provide us with GVT instrumentation to keep track of global time throughout the simulation. It will also provide us with the event tracing capabilities. These capabilities will allow for each event to be kept track of so that computation can be executed at designated times within the simulation. The goal of this project is to build upon the existing work of Deelman and Szymanski, whose paper is linked below. In their model, they considered the spread of Lyme Disease through ticks and the movement of mice; we would like to also add deer to the model and create events tailored to the spread of Lyme Disease through deer.  In addition, we would like to see how this addition of deer will change our results in comparison to the results shared in the paper.
Initially, the model considers ticks at the early life stages. When a tick hatches, it is not infected because ticks cannot pass the disease onto their eggs. When the tick becomes a larva, it will find blood from the common white-footed mouse; if the mouse is infected, the larva will be infected as well. Next, the larva will drop off their host and become a nymph, which remains dormant during the winter. In the spring, the nymph will become active again and feed on mice. If the tick has previously been infected, it can now infect new hosts. The last stage is after its second blood meal, when a tick becomes an adult. In this stage, the tick feeds on larger animals, most commonly on white-tail deer. In our project, we will add the adult stage when modeling ticks; this includes defining an adult bite (along with the existing larval and nymphal). In addition, we will model the movement of deer and the percentage probability that the deer will be infected.
In terms of parallelizing the algorithm, we would like to split up the “space” between ranks. The simulation space will be a 2D array (wrapped in both directions), where each node represents the size of a mouse home range (400 m2). The space will be split into n sections when there are n processors used and each of the sections will use be assigned a logical process (LP); this closely models the paper cited below. 

From this paper, it is clear that improving performance through parallelization of this problem is not straightforward; therefore, we do not find this project to be “embarrassingly parallel”. From the paper, it was found that the speedup grew for up to 10 processors and then began decreasing afterwards. Later, to improve performance, the simulation space was divided into more sections and less area was assigned to each LP. This again changed the speedup of the simulation with varying numbers of LPs and processors. We hope to run various tests and find the configuration that results in the the maximum speedup for our model. 

## Link to ROSS ##

Link: http://carothersc.github.io/ROSS/

## Command line commands (..):

export PATH=/usr/local/mpich-3.2/bin:$PATH

PPCkima5@kratos:~/ross-build$ cmake -DROSS_BUILD_MODELS=ON -DCMAKE_C_COMPILER=`which mpicc` -DCMAKE_ARCH=x86_64 ../ROSS

mpirun -np 4 ./phold --synch=2  (2 = conservative = mpi barrier at lookahead)

## working on kratos

Mark as a point of contact for questions, email: plaggm@rpi.edu

can use clion, with mastiff to connect to kratos

## How do we make a 'make' file for our model?



