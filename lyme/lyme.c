/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<clcg4.h>
#include<mpi.h>
#include<pthread.h>

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

/***************************************************************************/
/* Thread Structs **********************************************************/
/***************************************************************************/

typedef struct threadInfo {		//struct defining the information for each thread

} thread;

typedef struct mouseInfo {
	int lifespan;				//lifespan of mouse
	int numDaysTraveled;		//number of days mouse has traveled
	int carrying;				//boolean if mice is carrying ticks
	int typeTickCarrying;
	int infected;

} mouse;

typedef struct deerInfo {
	int infected;
	int carrying;
} deer;

typedef struct nestInfo {		//struct for each cell in board
	int larva;					//counts of each type of tick
	int uninfectedNymph;
	int infectedNymph;
	int uninfectedAdult;
	int infectedAdult;
	int numMice;				//count for number of mice in cell
	int numDeer;				//count for number of deer in cell
	mouse * miceinNest;			//array of mice in nest
	deer * deerinNest;			//array of deer in nest
} nest;


/***************************************************************************/
/* Global Variables ********************************************************/
/***************************************************************************/

int days; 				//number of simulation days
int infectionRate; 		//infection rate
int numRanks;			//total number of ranks 
int myRank;				//rank number
int numThreadsPer;		//number of threads per rank
int universeSize;		//size of x and y dimensions of the universe
int numRowsPer;			//number of rows each rank is responsible for
nest ** universe;		//universe board
int miceTravelDays;		//number of days a mice can travel before it dies
int tickFeedingDays;	//number of days needed for tick to feed
int totalNumMouse; 		//total number of mice across the simulation
int totalNumDeer; 		//total number of deer across the simulation
int carryLarva;			//number of larva a mice carries when it gets bit
int carryNymph;			//number of nymphs a mice carries when it gets bit
int carryAdult;			//numer of adult ticks a mice carries when it gets bit


/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
void initUniverse();

/***************************************************************************/
/* Notes         ***********************************************************/
/***************************************************************************/

/* typeTickCarrying: 0 - larva
					 1 - uninfected nymph
					 2 - infected nymph
					 3 - uninfected adult
					 4 - infected adult

		   carrying: 0 - not carrying a tick
		   			 1 - carrying a tick

		   infected: 0 - not infected with Lyme disease
		   			 1 - infected with Lyme disease
*/

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char* argv[])
{

// Example MPI startup and using CLCG4 RNG
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &numRanks);
    MPI_Comm_rank( MPI_COMM_WORLD, &myRank);
    
// Init 16,384 RNG streams - each rank has an independent stream
    InitDefault();
    
    MPI_Barrier( MPI_COMM_WORLD );

// Read in command-line arguments
   	days = 1;
   	infectionRate = 25;
   	numThreadsPer = 0;
   	universeSize =  6;
   	miceTravelDays = 4;
   	tickFeedingDays = 3;
   	totalNumMouse = 9;
   	totalNumDeer = 2;
   	carryLarva = 10;
   	carryNymph = 5;
   	carryAdult = 2;

// Allocate my rank's chunk of universe
    int i=0;
    numRowsPer = universeSize / numRanks;
    universe = (nest **) malloc(numRowsPer * sizeof(nest *));
    for (i=0; i < numRowsPer; i++) {
    	universe[i] = (nest *) malloc(universeSize * sizeof(nest));
    }

//Initialize universe with ticks, mice, and deer
   initUniverse();

// Start timing
    double start = 0;
    double end = 0;
    if (myRank == 0) start = MPI_Wtime();


   MPI_Barrier( MPI_COMM_WORLD );

// End timing
   if (myRank == 0) end = MPI_Wtime();
   printf("Time taken: %lf s\n", end - start);

//Frees

}

void initUniverse() { 
	/*Comments: It's going to be hard to allocate a total number of mice across the entire
	universe, due to it being across ranks. I was thinking we create a Python program to do it, output 
	the allocations to a text file, and this program has each rank read its portion of allocations from the
	text file. */

}