/***************************************************************************/
/* lyme.c ******************************************************************/
/* By: Nickolas Fay, Sara Khedr, Anthony Kim, and Anders Maraviglia ********/
/***************************************************************************/


/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <clcg4.h>
#include <mpi.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <structs.h>
#include <mouse_list.h>
#include <nest_list.h>
#include <hwi/include/bqc/A2_inlines.h>


/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define TIME 0 // should be 0, 1 on local machines

/***************************************************************************/
/* Thread Structs **********************************************************/
/***************************************************************************/

/* Inside structs.h */

/***************************************************************************/
/* Global Variables ********************************************************/
/***************************************************************************/


int days; 				//number of simulation days
float biteThreshold; 		//infection rate
int numRanks;			//total number of ranks 
int myRank;				//rank number
int pthreads;		//number of threads per rank
int universeSize;		//size of x and y dimensions of the universe
int numRowsPer;			//number of rows each rank is responsible for
int miceTravelDays;		//number of days a mice can travel before it dies
int tickFeedingDays;	//number of days needed for tick to feed
int totalNumMouse; 		//total number of mice across the simulation
int totalNumDeer; 		//total number of deer across the simulation
int carryLarva;			//number of larva a mice carries when it gets bit
int carryNymph;			//number of nymphs a mice carries when it gets bit
int carryAdult;			//number of adult ticks a mice carries when it gets bit
int tickBand;			//column band size of where ticks are initialized
int mouseThreshold;		//number of mice that can live in a nest
int rowLowerBound;		//lower bound of a rank's portion of the universe
int rowUpperBound;		//upper bound of a rank's portion of the universe
int numMicePerNest; 	//number of mice initialized in each spot within chckerboard structure
int mouseLifespan;		//lifespan of mouse
int uninfectedNymph;	//number of ticks initialized in each spot
int larvaSpawnDay;		//day that larva eggs hatch
int numLarva;			//number of larva that hatch on the "Spawn Day"
int prevRankID;
int nextRankID;
int mouseUID_cntr;
int maxFeedableMicePerNest;
int bgClock;

unsigned long long rankCommunicationTime;
unsigned long long maxRankCommunicationTime;
unsigned long long nestListArrCount;
unsigned long long mouseListArrCount;
unsigned long long newMouseListArrCount;

pthread_mutex_t sendMiceUpperMutex;
pthread_mutex_t sendMiceLowerMutex;
pthread_mutex_t nestListArrCountMutex; 
pthread_mutex_t mouseListArrCountMutex;
pthread_mutex_t newMouseListArrCountMutex;

nest *** universe;								//universe board

pthread_barrier_t barrier;						//barrier for threads

mouse_list * sendMiceUpper;						//list containing mice that leave rank's area from the bottom
mouse_list * sendMiceLower;						//list containing mice that leave rank's area from the top
mouse_list ** mouseListArr;
mouse_list ** newMouseListArr;
nest_list ** nestListArr;


/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
void initUniverse();
void initLists();
void computeTickBiteMouse(mouse* currMouse, nest* currNest, int currDay);
void moveMouse(mouse* currMouse, int currThread);
void computeTickDropoffMouse(mouse* currMouse, nest* currNest, int currDay);
int constructCommunicationArr(mouse_list * mList, int* commArr);
void addExternalMiceToRank(int* commArr, int commArrSize);
void communicateBetweenRanks();
void calcMouseDirection(mouse * m, int trueRow);
void initSingleMouse(mouse_list * miceInNest, nest *n, int trueRow, int j);
void initMouse(int i, int j, int trueRow);
void initTicks(int i, int j, int trueRow, int bandStart, int bandEnd);
void pthreadCreate();
void * updateUniverse(void *s);
void readCommandLineArgs(int argc, char* argv[]);
void printBoard();


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

int main(int argc, char* argv[]){
	// Example MPI startup and using CLCG4 RNG
	MPI_Init( &argc, &argv);
	MPI_Comm_size( MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank( MPI_COMM_WORLD, &myRank);
	
	// Init 16,384 RNG streams - each rank has an independent stream
	InitDefault();
	
	MPI_Barrier( MPI_COMM_WORLD );

	// Read in command-line arguments and set global variables
	readCommandLineArgs(argc, argv);

	// calculate neighbor rank ID's
	prevRankID = myRank-1;
	nextRankID = myRank+1;
	if(prevRankID == -1)
		prevRankID = numRanks-1;
	if(nextRankID == numRanks)
		nextRankID = 0;

	//Initialize universe with ticks, mice, and deer
	initUniverse();
	//printBoard();

	// Start timing
	double start = 0;
	double end = 0;
	if (myRank == 0) start = MPI_Wtime();

	// Create threads and run all iterations
	pthreadCreate();

	// Sync back all ranks
	MPI_Barrier( MPI_COMM_WORLD );

	MPI_Allreduce(&rankCommunicationTime, &maxRankCommunicationTime, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);

	// End timing
	if (myRank == 0)  {
		end = MPI_Wtime();
		printf("Config:\nUniverse Size: [%d, %d]\nRanks: [%d]\nThreads per Rank: [%d]\n", universeSize, universeSize, numRanks, pthreads);
		printf("Time taken: %lf s\n", end - start);
		printf("Rank communication took: %f s\n", ((float)maxRankCommunicationTime/(float)bgClock));
	}

	printBoard();

	// Frees
	for(int i =0; i < pthreads; i++){
		mouse_list_free(mouseListArr[i]);
		mouse_list_free(newMouseListArr[i]);
		nest_list_free(nestListArr[i]);
	}
	free(mouseListArr);
	free(newMouseListArr);
	free(nestListArr);
	pthread_mutex_destroy(&nestListArrCountMutex);
	pthread_mutex_destroy(&mouseListArrCountMutex);
	pthread_mutex_destroy(&newMouseListArrCountMutex);
	pthread_mutex_destroy(&sendMiceUpperMutex);
	pthread_mutex_destroy(&sendMiceLowerMutex);
	mouse_list_free(sendMiceUpper);
	mouse_list_free(sendMiceLower);

	for(int i = 0; i < numRowsPer; i++){
		for(int j = 0; j < universeSize; j++){
			pthread_mutex_destroy(&(universe[i][j]->mutex));
			mouse* currMouse;
			if (universe[i][j]->numMice > 0){
				//printf("rank[%d] pop_mouse_left 3\n", myRank);
				while((currMouse = pop_mouse_left(universe[i][j]->miceInNest)) != NULL){
					//printf("freeing mouse [%d] which lives in location [(%d,%d)] and should be in nest [(%d,%d)]\n", currMouse->mouseUID, currMouse->nextHome_x, currMouse->nextHome_y, currMouse->currentNest->i, currMouse->currentNest->j);
					currMouse->currentNest = NULL;
					if(currMouse->nextHome_x != universe[i][j]->i || currMouse->nextHome_y != j)
						printf("rank [%d]: freeing mouse [%d] which should be in location [(%d,%d)] but is in location [(%d,%d)]\n",
							myRank, currMouse->mouseUID, currMouse->nextHome_x, currMouse->nextHome_y, universe[i][j]->i, j);
					free(currMouse);
				}
				mouse_list_free(universe[i][j]->miceInNest);
			}
			free(universe[i][j]);
		}
	}
	MPI_Barrier( MPI_COMM_WORLD );
	MPI_Finalize();
	return 0;
}


//This function sets the global variables for the simulation that are described above in the global variables section. It can be through command-line arguments or the variables that are set here.
void readCommandLineArgs(int argc, char* argv[]){
	days = 180;
	biteThreshold = 0.5;
	universeSize = 0; 		//set by command line
	miceTravelDays = 5;
	tickFeedingDays = 3;
	carryLarva = 10;
	carryNymph = 5;
	carryAdult = 2;
	pthreads = 1;			//set by command line
	mouseThreshold = 1;
	numMicePerNest = 8;
	mouseLifespan = 200;
	uninfectedNymph = 1000;
	larvaSpawnDay = 90; 
	numLarva = 1000;
	maxFeedableMicePerNest = 5;
	nestListArrCount = 0;
	mouseListArrCount = 0;
	newMouseListArrCount = 0;
	mouseUID_cntr = 0;
	rankCommunicationTime = 0;
	bgClock = 1600000000;
	maxRankCommunicationTime = 0;

	if(argc >= 3){ // order of arguments doesn't matter, and we do not require arguments
		for(int currArg = 1; currArg < argc; currArg+=2){ 
			if(currArg+1 < argc){
				if(strcmp(argv[currArg], "--pthreads") == 0){ // to add another command line arg, just add an 'else if' of this format 
					pthreads = atoi(argv[currArg+1]);
				} else if(strcmp(argv[currArg], "--universeSize") == 0){
					universeSize = atoi(argv[currArg+1]);
				} 
			}	
		}
	}

	tickBand = universeSize * 0.25;
	numRowsPer = universeSize / numRanks;
	rowLowerBound = numRowsPer * myRank;
	rowUpperBound = numRowsPer * (myRank+1);
}

//This function computes whether a mouse gets bit by a tick and if so, which tick it gets bit by. The type of tick it gets bit by is based on the distribution of ticks within the nest.
void computeTickBiteMouse(mouse* currMouse, nest* currNest, int currDay){ 
	float mouse_gets_bit = GenVal(currMouse->nextHome_x);
	int totTicks = currNest->larva + currNest->uninfectedNymph + currNest->infectedNymph;
	if(mouse_gets_bit > biteThreshold || currMouse->carrying == 1 || totTicks == 0) // then the mouse gets off scott free
		return; 
	float random_val = GenVal(currMouse->nextHome_x);
	float probLarva = (float)currNest->larva / (float)totTicks;
	float probUninfectedNymph = (float)currNest->uninfectedNymph / (float)totTicks;
	if(random_val <= probLarva){ // if 0 <= random_val <= probLarva, then bit by larva
		currMouse->typeTickCarrying = 0;
		currNest->larva -= carryLarva; 
	} else if(probLarva < random_val && random_val <= probLarva + probUninfectedNymph){ // bit by uninfected nymph
		currMouse->typeTickCarrying = 1;
		currNest->uninfectedNymph -= carryNymph;
	} else{ // then get bit by an infected nymph
		currMouse->typeTickCarrying = 2;
		currNest->infectedNymph -= carryNymph;
		currMouse->infected = 1;
	}
	currMouse->carrying = 1; // the mouse is now carrying ticks
	currMouse->tickDropOffDate = currDay + tickFeedingDays; // set the day the ticks should get dropped off
}

/*This function computes where a mouse should move next based on it's x and y direction. If the mouse leaves the rank's area
 from the bottom, it is sent to the next rank (myRank + 1) and if it is leaves from the top, it sent to the previous rank (myRank -1).
 If it stays within the rank's area, it gets moved.*/
void moveMouse(mouse* currMouse, int currThread){
	int newXPos = currMouse->nextHome_x + currMouse->direction_x;
	int newYPos = currMouse->nextHome_y + currMouse->direction_y;
	if (newYPos == universeSize) {
		newYPos = 0;
	} else if (newYPos == -1) {
		newYPos = universeSize - 1;
	}
	if(newXPos == -1){
		currMouse->nextHome_x = universeSize - 1;
	} else if(newXPos == universeSize){
		currMouse->nextHome_x = 0;
	} else
		currMouse->nextHome_x = newXPos;
	currMouse->nextHome_y = newYPos;
	if(newXPos < rowLowerBound){ // if mouse must go to myRank - 1
		pthread_mutex_lock(&sendMiceLowerMutex); // two threads may try to move two mice out of the rank at the same time
		mouse_list_add_element(sendMiceLower, currMouse);
		pthread_mutex_unlock(&sendMiceLowerMutex);
	} else if(newXPos >= rowUpperBound){ // if mouse must go to myRank + 1
		pthread_mutex_lock(&sendMiceUpperMutex);
		mouse_list_add_element(sendMiceUpper, currMouse);
		pthread_mutex_unlock(&sendMiceUpperMutex);
	} else{ // normal move
		int x = currMouse->nextHome_x - rowLowerBound;
		pthread_mutex_lock(&(universe[x][currMouse->nextHome_y]->mutex)); // lock the nest
		mouse_list_add_element(universe[x][currMouse->nextHome_y]->miceInNest, currMouse); // add mouse to new nest
		universe[x][currMouse->nextHome_y]->numMice++; // nestListArrCount
		if (universe[x][currMouse->nextHome_y]->inANestList != 1) { // if the nest hasn't been added to any nest list, add it to the nest threads nest list
			universe[x][currMouse->nextHome_y]->inANestList = 1;
			nest_list_add_element(nestListArr[currThread], universe[x][currMouse->nextHome_y]);
		}
		pthread_mutex_unlock(&(universe[x][currMouse->nextHome_y]->mutex)); // safety
		currMouse->currentNest = universe[x][currMouse->nextHome_y]; // set mouse to nest backpointer
		mouse_list_add_element(newMouseListArr[currThread], currMouse); // add the mouse to the ranks mouse list
	}
}

//Computes whether a mouse drops off ticks into the nest it is currently in; also computes whether the tick got infected.
void computeTickDropoffMouse(mouse* currMouse, nest* currNest, int currDay){
	if(currMouse->carrying == 0) // if mouse isn't carrying ticks
		return;
	if(currMouse->tickDropOffDate != currDay)
		return;
	if(currMouse->typeTickCarrying == 0){ // mouse is carrying larva
		if(currMouse->infected == 1){ // if the mouse is infected, drop off infected nymphs
			currNest->infectedNymph += carryLarva;
		} else
			currNest->uninfectedNymph += carryLarva;
	} else if(currMouse->typeTickCarrying == 1){ // mouse carrying uninfected nymphs
		if(currMouse->infected == 1){ // if the mouse is infected, drop off infected adults
			currNest->infectedAdult += carryNymph;
		} else
			currNest->uninfectedAdult += carryNymph;
	} else if(currMouse->typeTickCarrying == 2){ // mouse carrying infected nymphs
		currNest->infectedAdult += carryNymph; // then we always drop off infected adults
	}
	currMouse->typeTickCarrying = -1; //mouse is no longer carrying ticks
	currMouse->carrying = 0;
}

/*Creates 2D array object representing mice to be sent to neighboring ranks; these mice are leaving their
current rank's area and will be send via I_send*/
int constructCommunicationArr(mouse_list * mList, int* commArr){
	int i = 0;
	if(mList->count > 0){
		mouse* currMouse;
		while((currMouse = pop_mouse_left(mList)) != NULL){
			commArr[i] = currMouse->lifespan;
			commArr[i+1] = currMouse->numDaysTraveled;
			commArr[i+2] = currMouse->carrying;
			commArr[i+3] = currMouse->typeTickCarrying;
			commArr[i+4] = currMouse->infected;
			commArr[i+5] = currMouse->tickDropOffDate;
			commArr[i+6] = currMouse->mustMove;
			commArr[i+7] = currMouse->nextHome_x;
			commArr[i+8] = currMouse->nextHome_y;
			commArr[i+9] = currMouse->direction_x;
			commArr[i+10] = currMouse->direction_y;
			commArr[i+11] = currMouse->mouseUID;
			i += 12;
		}
	}
	return i;
}

/* These mice are entering this rank's area via a neighboring rank; the received 2D array is used to reconstruct mouse objects */
void addExternalMiceToRank(int* commArr, int commArrSize){
	for(int i = 0; i < commArrSize; i+=12){
		mouse * newMouse = (mouse *)  malloc(sizeof(mouse));
		newMouse->lifespan = commArr[i];
		newMouse->numDaysTraveled = commArr[i+1];
		newMouse->carrying = commArr[i+2];
		newMouse->typeTickCarrying = commArr[i+3];
		newMouse->infected = commArr[i+4];
		newMouse->tickDropOffDate = commArr[i+5];
		newMouse->mustMove = commArr[i+6];
		newMouse->nextHome_x = commArr[i+7];
		newMouse->nextHome_y = commArr[i+8];
		newMouse->direction_x = commArr[i+9];
		newMouse->direction_y = commArr[i+10];
		newMouse->mouseUID = commArr[i+11];
		int x = newMouse->nextHome_x - rowLowerBound;
		mouse_list_add_element(universe[x][newMouse->nextHome_y]->miceInNest, newMouse); // add mouse to new nest
		universe[x][newMouse->nextHome_y]->numMice++; // nestListArrCount
		if (universe[x][newMouse->nextHome_y]->inANestList != 1) { // if the nest hasn't been added to any nest list, add it to the nest threads nest list
			pthread_mutex_lock(&nestListArrCountMutex); // ensure that 2 threads cannot add this nest back at the same time
			int nestIndex = nestListArrCount % pthreads;
			nestListArrCount++;
			nest_list_add_element(nestListArr[nestIndex], universe[x][newMouse->nextHome_y]); // add the nest to the ranks nest list
			pthread_mutex_unlock(&nestListArrCountMutex);
			universe[x][newMouse->nextHome_y]->inANestList = 1;
		}
		newMouse->currentNest = universe[x][newMouse->nextHome_y]; // set mouse to nest backpointer
		int mouseIndex = mouseListArrCount % pthreads;
		mouseListArrCount++;
		mouse_list_add_element(mouseListArr[mouseIndex], newMouse); // add the mouse to the ranks mouse list
	}
}

/* This function handles the communication between neighboring ranks. Neighboring ranks send mice that have left their area via a constructed
2D array that represents each mice via a row; after this is received, the rank reconstructs mouse objects via the 2D array */
void communicateBetweenRanks(int currDay){
	int* lowerCommArr = NULL, * upperCommArr = NULL;
	int sizeLowerIncomingCommArr, sizeUpperIncomingCommArr;
	lowerCommArr = (int*)malloc(sendMiceLower->count*12*sizeof(int));
	upperCommArr = (int*)malloc(sendMiceUpper->count*12*sizeof(int*));
	int sizeLowerCommArr = constructCommunicationArr(sendMiceLower, lowerCommArr);
	int sizeUpperCommArr = constructCommunicationArr(sendMiceUpper, upperCommArr);
	MPI_Request request1, request2, request3, request4, request5, request6, request7, request8;
	MPI_Status status1, status2, status3, status4;
	// MPI Isends
	MPI_Isend(&sizeUpperCommArr, 1, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request4); // send sendMiceUpper->count
	MPI_Isend(upperCommArr, sizeUpperCommArr, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request3); // send upperCommArr
	MPI_Isend(&sizeLowerCommArr, 1, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request2); // send sendMiceLower->count
	MPI_Isend(lowerCommArr, sizeLowerCommArr, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request1); // send lowerCommArr
	// MPI Ireceives
	MPI_Irecv(&sizeLowerIncomingCommArr, 1, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request5);
	MPI_Wait(&request5, &status1);
	int incomingLowerCommArr[sizeLowerIncomingCommArr];
	MPI_Irecv(incomingLowerCommArr, sizeLowerIncomingCommArr, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request6);
	MPI_Wait(&request6, &status2);
	MPI_Irecv(&sizeUpperIncomingCommArr, 1, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request7);
	MPI_Wait(&request7, &status3);
	int incomingUpperCommArr[sizeUpperIncomingCommArr];
	MPI_Irecv(incomingUpperCommArr, sizeUpperIncomingCommArr, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request8);
	MPI_Wait(&request8, &status4);
	addExternalMiceToRank(incomingLowerCommArr, sizeLowerIncomingCommArr); // add new mice to the rank
	addExternalMiceToRank(incomingUpperCommArr, sizeUpperIncomingCommArr);
}

/* Adds larva in a checkerboard fashion where the over-wintered nymphs were originally placed */
void addLarva(){
	int trueRow = myRank * numRowsPer;
	int bandStart = universeSize/2 - tickBand/2;
	int bandEnd = universeSize/2 + tickBand/2;
	int i=0;
	int j=0;
	for (i=0; i < numRowsPer; i++) {
		for (j=0; j < universeSize; j++) {
			if (trueRow % 4 == 0 && j >=bandStart && j < bandEnd) // then there must be larva in this cell
				universe[i][j]->larva = numLarva;	
		}
		trueRow++;
	}
}

/* This function takes care of updating the board; it processes a list of nests with mice to update tick bites and future mouse movements.
Then it processes all mice, checking lifespan and updating their next movement if they must leave their current nest */
void * updateUniverse(void *s) {
	thread * t = s;
	for(int currDay = 0; currDay < days; currDay++){
		nest* currNest;
		while((currNest = pop_nest_left(nestListArr[*t->myTID])) != NULL){ // go over all of this threads nests with mice (in parallel since we are in a thread)
			currNest->inANestList = 0; // reset nest list
			mouse* currMouse;
			int mouseCntr = 0;
			mouse_list * miceStillInNest = mouse_list_create();
			while((currMouse = pop_mouse_left(currNest->miceInNest)) != NULL){ // go over all mice in current nest
				if(currMouse->lifespan > currDay && currMouse->numDaysTraveled < miceTravelDays){ // if the mouse's time isn't up...	
					computeTickDropoffMouse(currMouse, currNest, currDay); // process ticks dropping off incoming mice
					if(mouseCntr < mouseThreshold){ // the first mouse may stay in the nest
						currMouse->mustMove = 0;
						mouse_list_add_element(miceStillInNest, currMouse);
					} else{ // send the rest packing, only mice that are leaving may be bit by ticks
						currMouse->mustMove = 1;
						if(mouseCntr > maxFeedableMicePerNest)
							currMouse->numDaysTraveled++;
						computeTickBiteMouse(currMouse, currNest, currDay); // process if this mouse get bit by ticks
					}
					mouseCntr++;
				}
			}
			mouse_list * temp = currNest->miceInNest;
			currNest->miceInNest = miceStillInNest;
			mouse_list_free(temp);
			temp = NULL;
			currNest->numMice = currNest->miceInNest->count;

		}

		if(*t->myTID == 0)
			nestListArrCount = 0; // reset nest list array counter here prep for adding nests back into nestListArr
		pthread_barrier_wait(&barrier); // wait for all nests to be computed
		mouse* currMouse;
		while((currMouse = pop_mouse_left(mouseListArr[*t->myTID])) != NULL){
			if(currMouse->lifespan == currDay){ // if the mouse's time is up...
				currMouse->currentNest = NULL; // mouse bites the dust
				free(currMouse);
			} else if (currMouse->mustMove == 1) { // else, mouse moves
				moveMouse(currMouse, *t->myTID);
			} else if (currMouse->mustMove == 0) {	//if not moving, still add to mouseList
				mouse_list_add_element(newMouseListArr[*t->myTID], currMouse);
			}
		}
		pthread_barrier_wait(&barrier); // set each threads mouse list for the nest iteration 
		mouse_list_free(mouseListArr[*t->myTID]); // free the current threads list ASAP
		mouseListArr[*t->myTID] = newMouseListArr[*t->myTID];
		newMouseListArr[*t->myTID] = NULL;
		newMouseListArr[*t->myTID] = mouse_list_create();
		pthread_barrier_wait(&barrier);
		if(*t->myTID == 0){ // only allow thread 0 to do MPI communication
			mouseListArrCount = newMouseListArrCount;
			newMouseListArrCount = 0;
			unsigned long long startCommCycles, endCommCycles;//time rank communication
			if (TIME == 0) startCommCycles = GetTimeBase();
			communicateBetweenRanks(currDay);
			if (TIME == 0){ 
				endCommCycles = GetTimeBase();
				rankCommunicationTime += (endCommCycles - startCommCycles);
			}
			if(currDay == larvaSpawnDay) // if its day zero and thread zero, break out the larva
				addLarva();
			if(currDay == 10) printBoard();
			if(currDay == 90) printBoard();
		}
		pthread_barrier_wait(&barrier);
	}
	return NULL;
}

/* Creates specified number of pthreads and has them do the update universe function */
void pthreadCreate() {
	pthread_barrier_init(&barrier, NULL, pthreads); //Initialize barrier
	pthread_t tid[pthreads]; //Create array holding total number of threads
	int i=0;
	for (i=0; i < pthreads; i++) {
		thread * t = (thread *)  malloc(sizeof(thread));
		int * id = (int *) malloc(sizeof(int)); //Assign thread its given ID
		*id = i;
		t->myTID = id;
		int rc = pthread_create( &tid[i], NULL, updateUniverse, t);
		if (rc != 0) {
			fprintf( stderr, "pthread_create() failed (%d): %s\n", rc, strerror( rc ));
			return;
		}
	}
	for (i=0; i < pthreads; i++) { //Wait for threads to join the main
		unsigned int * x;
		pthread_join(tid[i], (void **) &x);
	}
}

/* Mouse can move in 1 of 8 directions; calculates direction randomly and assigns appropriate x and y direction
to mouse */
void calcMouseDirection(mouse * m, int trueRow){
	float random_val = GenVal(trueRow);
	float aFraction = ((float)1/(float)8);
	int direction = (int)(random_val / aFraction);
	if(direction == 0){
		m->direction_x = -1;
		m->direction_y = 0;
	} else if(direction == 1){
		m->direction_x = -1;
		m->direction_y = 1;
	} else if(direction == 2){
		m->direction_x = 0;
		m->direction_y = 1;
	} else if(direction == 3){
		m->direction_x = 1;
		m->direction_y = 1;
	} else if(direction == 4){
		m->direction_x = 1;
		m->direction_y = 0;
	} else if(direction == 5){
		m->direction_x = 1;
		m->direction_y = -1;
	} else if(direction == 6){ 
		m->direction_x = 0;
		m->direction_y = -1;
	} else{
		m->direction_x = -1;
		m->direction_y = -1;
	}
}

/* Initializes single mouse with all information in the struct */
void initSingleMouse(mouse_list * miceInNest, nest *n, int trueRow, int j){
	mouse* m = (mouse *)  malloc(sizeof(mouse));
	m->lifespan = mouseLifespan;
	m->numDaysTraveled = 0;
	m->carrying = 0;
	m->typeTickCarrying = -1;
	m->infected = 0;
	m->currentNest = n;
	m->nextHome_x = trueRow;
	m->nextHome_y = j;
	m->tickDropOffDate = -1;  // should be -1 if there are no ticks on this mouse
	mouse_list_add_element(miceInNest, m);
	int mIndex = mouseListArrCount % pthreads;
	mouse_list_add_element(mouseListArr[mIndex], m);
	m->mouseUID = myRank * 10000 + mouseUID_cntr;
	mouseUID_cntr++;
	mouseListArrCount++;
	m->mustMove = 1; // mark all mice to move initally
	calcMouseDirection(m, trueRow); // give the mouse a direction to go in
}

/*Initializes mice in a checkerboard fashion on the board */
void initMouse(int i, int j, int trueRow) {
	nest *n = (nest *)malloc(sizeof(nest));
	pthread_mutex_init(&(n->mutex), NULL);
	if (trueRow % 2 == 0) {
		if (j % 2 == 0) {
			n->numMice = numMicePerNest; //make Mice objects
			mouse_list * miceInNest = mouse_list_create();
			for(int k = 0; k < numMicePerNest; k++)
				initSingleMouse(miceInNest, n, trueRow, j);
			n->miceInNest = miceInNest;
			int nIndex = nestListArrCount % pthreads;
			nest_list_add_element(nestListArr[nIndex], n);
			nestListArrCount++;
		} else {
			n->numMice = 0;
			n->miceInNest = mouse_list_create(); // note: this may haveto change
		}
	} else {
		if (j % 2 == 1) {
			n->numMice = numMicePerNest; //make Mice objects
			mouse_list * miceInNest = mouse_list_create();
			for(int k = 0; k < numMicePerNest; k++)
				initSingleMouse(miceInNest, n, trueRow, j);
			n->miceInNest = miceInNest;
			int nIndex = nestListArrCount % pthreads;
			nest_list_add_element(nestListArr[nIndex], n);
			nestListArrCount++;
		}
		else {
			n->numMice = 0;
			n->miceInNest = mouse_list_create(); // note: this may haveto change
		}
	}
	universe[i][j] = n;
}

// Initializes nymphs within a column band, every fourth row
void initTicks(int i, int j, int trueRow, int bandStart, int bandEnd) {
	universe[i][j]->larva = 0;
	universe[i][j]->infectedNymph = 0;
	universe[i][j]->uninfectedAdult = 0;
	universe[i][j]->infectedAdult = 0;
	universe[i][j]->i = trueRow;
	universe[i][j]->j = j;
	universe[i][j]->inANestList = 0;
	if (trueRow % 4 == 0 && j >=bandStart && j < bandEnd) { // then there are ticks in this cell
		if(trueRow % (universeSize/2) == 0 && trueRow != 0){ // then this tick nest has lyme disease
			universe[i][j]->infectedNymph = uninfectedNymph/2;
			universe[i][j]->uninfectedNymph = uninfectedNymph/2;
		} else{
			universe[i][j]->uninfectedNymph = uninfectedNymph;
		}
	} else // otherwise it does not
		universe[i][j]->uninfectedNymph = 0;
	if((j == universeSize/2 && trueRow >= bandStart && trueRow < bandEnd) && j >=bandStart && j < bandEnd){
		universe[i][j]->infectedNymph = uninfectedNymph/2;
		universe[i][j]->uninfectedNymph = uninfectedNymph/2;
	}
}

/* Initializes all global lists needed */
void initLists(){
	sendMiceLower = mouse_list_create();
	sendMiceUpper = mouse_list_create();
	nestListArr = (nest_list**)malloc(pthreads*sizeof(nest_list*));
	mouseListArr = (mouse_list**)malloc(pthreads*sizeof(mouse_list*));
	newMouseListArr = (mouse_list**)malloc(pthreads*sizeof(mouse_list*));
	for(int i = 0; i < pthreads; i++){
		nestListArr[i] = nest_list_create();
		mouseListArr[i] = mouse_list_create();
		newMouseListArr[i] = mouse_list_create();
	}
	pthread_mutex_init(&sendMiceUpperMutex, NULL);
	pthread_mutex_init(&sendMiceLowerMutex, NULL);
	pthread_mutex_init(&nestListArrCountMutex, NULL);
	pthread_mutex_init(&mouseListArrCountMutex, NULL);
	pthread_mutex_init(&newMouseListArrCountMutex, NULL);
}

// Initializes ticks and mice
void initUniverse() {
	int trueRow = myRank * numRowsPer;
	int bandStart = universeSize/2 - tickBand/2;
	int bandEnd = universeSize/2 + tickBand/2;
	int i=0;
	int j=0;
	initLists();
	universe = (nest ***) malloc(numRowsPer * sizeof(nest **));
	int totNumMice = 0;
	for (i=0; i < numRowsPer; i++) {
		universe[i] = (nest **) malloc(universeSize * sizeof(nest*));
		for (j=0; j < universeSize; j++) {
			initMouse(i, j, trueRow);
			initTicks(i, j, trueRow, bandStart, bandEnd);
			if(universe[i][j]->numMice > 0) 
				totNumMice+=(universe[i][j]->numMice);
		}
		trueRow ++;
	}
}

void printBoard() {
    char rankNum[256];
    sprintf(rankNum, "%d", myRank);
    char fname[256] = "result.";
    strcat(fname, rankNum);
    FILE * f = fopen(fname, "a");
    int i=0;
    int j=0;
    for (i=0; i < numRowsPer; i++) {
        for (j=0; j < universeSize; j++) {
        	//int total = universe[i][j]->larva + universe[i][j]->uninfectedNymph + universe[i][j]->infectedNymph
        	//+ universe[i][j]->uninfectedAdult + universe[i][j]->infectedAdult;
        	//int micePerCell = universe[i][j]->miceInNest->count;
        	//int infected = universe[i][j]->infectedNymph + universe[i][j]->infectedAdult;
        	int infectedMice = 0;
        	if(universe[i][j]->miceInNest->count > 0){
        		mouse_list * tempMouseList = mouse_list_create();
        		mouse* currMouse;
        		while((currMouse = pop_mouse_left(universe[i][j]->miceInNest)) != NULL){
        			if(currMouse->infected)
        				infectedMice++;
        			mouse_list_add_element(tempMouseList, currMouse);
        		}
        		mouse_list_free(universe[i][j]->miceInNest);
        		universe[i][j]->miceInNest = NULL;
        		universe[i][j]->miceInNest = tempMouseList;
        	}
        	// float infectionRate = (float)(infected)/(float)(total);
         	fprintf(f, "%d ", infectedMice);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
    fclose(f);
}