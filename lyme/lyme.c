/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include <stdio.h>
// Make configuration structures within separate file (.h) for arguments

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <clcg4.h>
#include <mpi.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>

#include <config.h>

#include <structs.h>
#include <mouse_list.h>
#include <nest_list.h>


/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

/***************************************************************************/
/* Thread Structs **********************************************************/
/***************************************************************************/

/* Inside structs.h */


/***************************************************************************/
/* Global Variables ********************************************************/
/***************************************************************************/

/*
int days; 				//number of simulation days
int infectionRate; 		//infection rate
int numRanks;			//total number of ranks 
int myRank;				//rank number
int pthreads;			//number of threads per rank
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

*/

nest *** universe;								//universe board
pthread_barrier_t barrier;						//barrier for threads
nest_list * nestList;							//list containing nests that have mice
mouse_list * mouseList;							//list containing all mice
mouse_list * newMouseList;
mouse_list * sendMiceUpper;						//list containing mice that leave rank's area from the bottom
mouse_list * sendMiceLower;						//list containing mice that leave rank's area from the top
int mouseUID_cntr = 0;



/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
void initUniverse();
void initLists();
void computeTickBiteMouse(mouse* currMouse, nest* currNest, int currDay);
void moveMouse(mouse* currMouse);
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
void testLists();



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
	initConfigs();
	
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

//Uncomment below line if wish to test lists  	
	//testLists();

//Initialize universe with ticks, mice, and deer
	initUniverse();

// Start timing
	double start = 0;
	double end = 0;
	if (myRank == 0) start = MPI_Wtime();

// run all iterations
	pthreadCreate();

// sync back all ranks
	MPI_Barrier( MPI_COMM_WORLD );

// End timing
	if (myRank == 0)  {
		end = MPI_Wtime();
		printf("Time taken: %lf s\n", end - start);
	}

//Frees
	mouse_list_free(mouseList);
	// remember to free all mouse lists in nest structs as well (didn't work on first attempt)
	//nest* currNest;
	//while((currNest = pop_nest_left(nestList)) != NULL){
	//	if(currNest->numMice > 0)
	//		mouse_list_free(currNest->miceInNest);
	//}
	nest_list_free(nestList);
	for(int i = 0; i < numRowsPer; i++){
		for(int j = 0; j < universeSize; j++){
			pthread_mutex_destroy(&(universe[i][j]->mutex));
			mouse* currMouse;
			if(universe[i][j]->numMice > 0){
				while((currMouse = pop_mouse_left(universe[i][j]->miceInNest)) != NULL){
					currMouse->currentNest = NULL;
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

void readCommandLineArgs(int argc, char* argv[]){
	
	days = 5;
	biteThreshold = .25;
	pthreads = 0;
	universeSize = 16;
	miceTravelDays = 4;
	tickFeedingDays = 3;
	totalNumMouse = 9;
	totalNumDeer = 2;
	carryLarva = 10;
	carryNymph = 5;
	carryAdult = 2;
	tickBand = 2;
	pthreads = 2;
	mouseThreshold = 1;
	numRowsPer = universeSize / numRanks;
	rowLowerBound = numRowsPer * myRank;
	rowUpperBound = numRowsPer * (myRank+1);
	numMicePerNest = 5;
	mouseLifespan = 100;
	uninfectedNymph = 1000;
	larvaSpawnDay = 3; //90;
	numLarva = 1000;
	//printf("rank[%d] numRowsPer[%d] \n", myRank, numRowsPer);

	if(argc >= 3){ // order of arguments doesn't matter, and we do not require arguments
		for(int currArg = 1; currArg < argc; currArg+=2){ 
			if(currArg+1 < argc){
				//printf("currArg[%d] argc[%d] arg_str[%s] val_str[%s]\n", currArg, argc, argv[currArg], argv[currArg+1]);
				if(strcmp(argv[currArg], "--pthreads") == 0){ // to add another command line arg, just add an 'else if' of this format 
					pthreads = atoi(argv[currArg+1]);
				} else if(strcmp(argv[currArg], "--universeSize") == 0){
					universeSize = atoi(argv[currArg+1]);
				} else if(strcmp(argv[currArg], "--config") == 0){ // keep only this 'if' when we switch to using config structs
					int configNum = atoi(argv[currArg+1]);
					useConfiguration(configNum);
				}
			}	
		}
	}
	//printf("configuration: pthreads per rank[%d] universeSize[%d]x[%d]\n", pthreads, universeSize, universeSize);
}

void computeTickBiteMouse(mouse* currMouse, nest* currNest, int currDay){ // mark t to remove after code works
	float mouse_gets_bit = GenVal(currMouse->nextHome_x);
	int totTicks = currNest->larva + currNest->uninfectedNymph + currNest->infectedNymph;
	if(mouse_gets_bit > biteThreshold || currMouse->carrying == 1 || totTicks == 0) // then the mouse get off scott free
		return; 
	// so this mouse DOES get bit
	float random_val = GenVal(currMouse->nextHome_x);
	//printf("rank[%d] thread[%d] mouse[%d] row[%d] random_val[%f]\n", myRank, *t->myTID, currMouse->mouseUID, currMouse->nextHome_x, random_val);
	
	pthread_mutex_lock(&(currNest->mutex)); // safety
	float probLarva = (float)currNest->larva / (float)totTicks;
	float probUninfectedNymph = (float)currNest->uninfectedNymph / (float)totTicks;
	//float probInfectedNymph = currNest->infectedNymph / totTicks;
	
	if(random_val <= probLarva){ // if 0 <= random_val <= probLarva, then bit by larva
		currMouse->typeTickCarrying = 0;
		currNest->larva -= carryLarva; // stuff will probs break here....
	} else if(probLarva < random_val && random_val <= probLarva + probUninfectedNymph){ // bit by uninfected nymph
		currMouse->typeTickCarrying = 1;
		currNest->uninfectedNymph -= carryNymph;
	} else{ // then get bit by an infected nymph
		currMouse->typeTickCarrying = 2;
		currNest->infectedNymph -= carryNymph;
		currMouse->infected = 1;
	}
	pthread_mutex_unlock(&(currNest->mutex));
	currMouse->carrying = 1; // the mouse is now carrying ticks
	currMouse->tickDropOffDate = currDay + tickFeedingDays; // set the day the ticks should get dropped off
}

void moveMouse(mouse* currMouse){
	int newXPos = currMouse->nextHome_x + currMouse->direction_x;
	int newYPos = currMouse->nextHome_y + currMouse->direction_y;
	//printf("rank[%d] newXPos[%d] rowLowerBound[%d] rowUpperBound[%d]\n", myRank, newXPos, rowLowerBound, rowUpperBound);
	// rowLowerBound, rowUpperBound
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
		//printf("rank[%d] newXPos[%d] rowLowerBound[%d]\n", myRank, newXPos, rowLowerBound);
		//currMouse->nextHome_x = newXPos;
		mouse_list_add_element(sendMiceLower, currMouse);
	} else if(newXPos >= rowUpperBound){ // if mouse must go to myRank + 1
		//currMouse->nextHome_x = newXPos;
		//printf("GOING UP rank[%d] mouse[%d] newXPos[%d] rowLowerBound[%d]\n", myRank, currMouse->mouseUID, newXPos, rowLowerBound);
		mouse_list_add_element(sendMiceUpper, currMouse);
	} else{ // normal move
		//reconstruct new nest queue and new mouse queue

		int x = currMouse->nextHome_x - rowLowerBound;
		mouse_list_add_element(universe[x][currMouse->nextHome_y]->miceInNest, currMouse);
		currMouse->currentNest = universe[x][currMouse->nextHome_y]; // set mouse to nest backpointer
		
		//Here is where all our dreams may die (<= fuckin sara...)
		universe[x][currMouse->nextHome_y]->numMice++;
		mouse_list_add_element(newMouseList, currMouse); // add the mouse to the ranks mouse list
		if (nest_list_contains_p(nestList, universe[x][currMouse->nextHome_y]) == 1) {
			nest_list_add_element(nestList, universe[x][currMouse->nextHome_y]); // add the nest to the ranks nest list 
		}
	}
}

void computeTickDropoffMouse(mouse* currMouse, nest* currNest, int currDay){
	if(currMouse->carrying == 0) // if mouse isn't carrying ticks, gtfo
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
}

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

void printMouse(mouse * m){
	printf("rank[%d] mouse[%d] loc[%d, %d]\n", myRank, m->mouseUID, m->nextHome_x, m->nextHome_y);
}

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
		mouse_list_add_element(universe[x][newMouse->nextHome_y]->miceInNest, newMouse);
		newMouse->currentNest = universe[x][newMouse->nextHome_y]; // set mouse to nest backpointer
		
		//Here is where all our dreams may die (<= fuckin sara...)
		universe[x][newMouse->nextHome_y]->numMice++;
		mouse_list_add_element(mouseList, newMouse); // add the mouse to the ranks mouse list
		if (nest_list_contains_p(nestList, universe[x][newMouse->nextHome_y]) == 1) {
			nest_list_add_element(nestList, universe[x][newMouse->nextHome_y]); // add the nest to the ranks nest list 
		}
	}
	
}

void communicateBetweenRanks(){
	int* lowerCommArr = NULL, * upperCommArr = NULL;
	//int* incomingLowerCommArr, * incomingUpperCommArr;
	int sizeLowerIncomingCommArr, sizeUpperIncomingCommArr;
	lowerCommArr = (int*)malloc(sendMiceLower->count*12*sizeof(int));
	upperCommArr = (int*)malloc(sendMiceUpper->count*12*sizeof(int*));
	int sizeLowerCommArr = constructCommunicationArr(sendMiceLower, lowerCommArr);
	int sizeUpperCommArr = constructCommunicationArr(sendMiceUpper, upperCommArr);
	//MPI_Isend();
	// we need to send the 2d arrays and the sizes of them (4 Isends, 4 Irecieves)
	MPI_Request request1, request2, request3, request4, request5, request6, request7, request8;
	MPI_Status status1, status2, status3, status4;

	//printf("rank[%d] OUTGOING sizeLowerCommArr[%d] sizeUpperCommArr[%d]\n", myRank, sizeLowerCommArr, sizeUpperCommArr);
	MPI_Isend(&sizeUpperCommArr, 1, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request4); // send sendMiceUpper->count
	MPI_Isend(upperCommArr, sizeUpperCommArr, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request3); // send upperCommArr
	//MPI_Wait(&request3, &status5);
	MPI_Isend(&sizeLowerCommArr, 1, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request2); // send sendMiceLower->count
	MPI_Isend(lowerCommArr, sizeLowerCommArr, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request1); // send lowerCommArr
	//MPI_Wait(&request1, &status6);

	// MPI Ireceives
	MPI_Irecv(&sizeLowerIncomingCommArr, 1, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request5);
	MPI_Wait(&request5, &status1);
	//printf("rank[%d] INCOMING sizeLowerIncomingCommArr[%d] \n", myRank, sizeLowerIncomingCommArr);
	//incomingLowerCommArr = (int*) malloc(sizeLowerIncomingCommArr*sizeof(int));
	int incomingLowerCommArr[sizeLowerIncomingCommArr];
	MPI_Irecv(incomingLowerCommArr, sizeLowerIncomingCommArr, MPI_INT, prevRankID, 0, MPI_COMM_WORLD, &request6);
	MPI_Wait(&request6, &status2);

	MPI_Irecv(&sizeUpperIncomingCommArr, 1, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request7);
	MPI_Wait(&request7, &status3);
	//printf("rank[%d] INCOMING sizeUpperIncomingCommArr[%d]\n", myRank, sizeUpperIncomingCommArr);
	//incomingUpperCommArr = (int*) malloc(sizeUpperIncomingCommArr*sizeof(int));
	int incomingUpperCommArr[sizeUpperIncomingCommArr];
	MPI_Irecv(incomingUpperCommArr, sizeUpperIncomingCommArr, MPI_INT, nextRankID, 0, MPI_COMM_WORLD, &request8);
	MPI_Wait(&request8, &status4);
	
	addExternalMiceToRank(incomingLowerCommArr, sizeLowerIncomingCommArr);
	addExternalMiceToRank(incomingUpperCommArr, sizeUpperIncomingCommArr);

}

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

void * updateUniverse(void *s) {
	thread * t = s;
	//printf("rank[%d] in thread[%d]\n", myRank, *t->myTID);
	// go over all nests with mice (in parallel)
	for(int currDay = 0; currDay < days; currDay++){
		nest* currNest;
		while((currNest = pop_nest_left(nestList)) != NULL){ // go over all nests with mice (in parallel since we are in a thread)
			//printf("rank[%d] thread[%d] on nest with [%d] mice\n",  myRank, *t->myTID, currNest->numMice);
			mouse* currMouse;
			int mouseCntr = 0;
			mouse_list * miceStillInNest = mouse_list_create();
			while((currMouse = pop_mouse_left(currNest->miceInNest)) != NULL){ // go over all mice in current nest
				if(currMouse->lifespan < currDay && currMouse->numDaysTraveled < miceTravelDays){ // if the mouse's time isn't up...	
					computeTickDropoffMouse(currMouse, currNest, currDay); // process ticks dropping off incoming mice
					// process if this mouse needs to move
					if(mouseCntr < mouseThreshold){ // the first mouse may stay in the nest
						currMouse->mustMove = 0;
						mouse_list_add_element(miceStillInNest, currMouse);
					} else{ // send the rest packing, only mice that are leaving may be bit by ticks
						currMouse->mustMove = 1;
						currMouse->numDaysTraveled++;
						computeTickBiteMouse(currMouse, currNest, currDay); // process if this mouse get bit by ticks
					}
					mouseCntr++;
				}
			}
			mouse_list_free(currNest->miceInNest);
			currNest->miceInNest = miceStillInNest;
			currNest->numMice = currNest->miceInNest->count;
		}
		pthread_barrier_wait(&barrier);
		// go over all mice in queue (in parallel)
		mouse* currMouse;
		while((currMouse = pop_mouse_left(mouseList)) != NULL){
			//printf("rank[%d] thread[%d] on mouse[%d] loc[%d][%d]\n",  myRank, *t->myTID, currMouse->mouseUID, currMouse->nextHome_x, currMouse->nextHome_y); // currMouse->mouseUID
			if(currMouse->lifespan == currDay){ // if the mouse's time is up...
				currMouse->currentNest = NULL; // kill dat bitch
				free(currMouse);
			} else{ // else, make them move dat bitch
				moveMouse(currMouse);
			}
		}
		pthread_barrier_wait(&barrier);
		//printf("rank[%d] thread[%d] after second barrier\n",  myRank, *t->myTID);
		if(*t->myTID == 0){ // only allow thread 0 to do MPI communication
			mouse_list_free(mouseList);
			mouseList = newMouseList;
			newMouseList = NULL; // simple sara...
			newMouseList = mouse_list_create();
			//newMouseList = NULL;
			printf("rank[%d] thread[%d] has [%d] mice, sending [%d] upper, sending [%d] mice lower\n", myRank, *t->myTID,
				mouseList->count, sendMiceUpper->count, sendMiceLower->count);
			communicateBetweenRanks();
			printf("rank[%d] thread[%d] numMiceInRank[%d] after iteration[%d]\n", myRank, *t->myTID, mouseList->count, currDay);
			if(currDay == larvaSpawnDay) // if its day zero and thread zero, break out the larva
				addLarva();
		}
		pthread_barrier_wait(&barrier);
		//printf("rank[%d] thread[%d] went over [%d] mice\n", myRank, *t->myTID, numMiceAccessed);
	}
	return NULL;
}

void pthreadCreate() {

	//Initialize barrier
	pthread_barrier_init(&barrier, NULL, pthreads);

	//Create array holding total number of threads
	pthread_t tid[pthreads];
	int i=0;
	for (i=0; i < pthreads; i++) {
		thread * t = (thread *)  malloc(sizeof(thread));

		//Assign thread its given ID
		int * id = (int *) malloc(sizeof(int));
		*id = i;
		t->myTID = id;
		int rc = pthread_create( &tid[i], NULL, updateUniverse, t);
		if (rc != 0) {
			fprintf( stderr, "pthread_create() failed (%d): %s\n", rc, strerror( rc ));
			return;
		}
	}

	//Wait for threads to join the main
	for (i=0; i < pthreads; i++) {
		unsigned int * x;
		pthread_join(tid[i], (void **) &x);
	}
}

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
	//printf("mouse.x[%d] mouse.y[%d]\n", *m.nextHome_x, *m.nextHome_y);
	m->tickDropOffDate = -1;  // should be -1 if there are no ticks on this mouse
	mouse_list_add_element(miceInNest, m);	
	mouse_list_add_element(mouseList, m);
	m->mouseUID = mouseUID_cntr;
	mouseUID_cntr++;
	m->mustMove = 1; // mark all mice to move initally
	// give the mouse a direction to go in
	calcMouseDirection(m, trueRow);
}

//Initializes mice in a checkerboard fashion
void initMouse(int i, int j, int trueRow) {
	nest *n = (nest *)malloc(sizeof(nest));
	pthread_mutex_init(&(n->mutex), NULL);
	if (trueRow % 2 == 0) {
		if (j % 2 == 0) {
			//make Mice objects
			n->numMice = numMicePerNest;
			mouse_list * miceInNest = mouse_list_create();
			for(int k = 0; k < numMicePerNest; k++){
				initSingleMouse(miceInNest, n, trueRow, j);
			}
			n->miceInNest = miceInNest;
			nest_list_add_element(nestList, n);
			//printf("should always be 0, [%d]\n", nest_list_contains_p(nestList, n));
		}
		else {
			n->numMice = 0;
			n->miceInNest = mouse_list_create(); // note: this may haveto change
		}
	}
	else {
		if (j % 2 == 1) {
			//make Mice objects
			n->numMice = numMicePerNest;
			mouse_list * miceInNest = mouse_list_create();
			for(int k = 0; k < numMicePerNest; k++){
				initSingleMouse(miceInNest, n, trueRow, j);
			}
			n->miceInNest = miceInNest;
			nest_list_add_element(nestList, n);
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
	if (trueRow % 4 == 0 && j >=bandStart && j < bandEnd) { // then there are ticks in this cell
		if(trueRow % (universeSize/2) == 0 && trueRow != 0){ // then this tick nest has lyme disease
			universe[i][j]->infectedNymph = uninfectedNymph/2;
			universe[i][j]->uninfectedNymph = uninfectedNymph/2;
			//printf("YAY infected ticks i[%d] j[%d]\n", trueRow,j);
		} else{
			universe[i][j]->uninfectedNymph = uninfectedNymph;
		}
	} else
		universe[i][j]->uninfectedNymph = 0;
}

void initLists(){
	nestList = nest_list_create();		//list containing nests that have mice
	mouseList = mouse_list_create();	//list containing all mice
	sendMiceLower = mouse_list_create();
	sendMiceUpper = mouse_list_create();
	newMouseList = mouse_list_create();
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
			// uncomment if we need to test list initalization
			
			if(universe[i][j]->numMice > 0){
				totNumMice+=(universe[i][j]->numMice);
				/*
				printf("rank[%d] i[%d] j[%d] num_mice[%d] should be at most [%d]\n", myRank, i, j, universe[i][j].numMice, numMicePerNest);
				for(int k = 0; k < universe[i][j].numMice; k++){

				}
				*/
			}
			
		}
		trueRow ++;
	}
	printf("rank[%d] totNumMice[%d] mouseList.count[%d] mouseUID_cntr[%d]\n", myRank, totNumMice, mouseList->count, mouseUID_cntr);
}


/***************************************************************************/
/* Other Functions  ********************************************************/
/***************************************************************************/

void testLists() {
	mouse_list* mltest = mouse_list_create();
	mouse one = { .lifespan = 10};
	mouse two = { .lifespan = 20};
	mouse three = { .lifespan = 30};
	mouse* aa = &one;
	mouse* bb = &two;
	mouse* cc = &three;
	mouse_list_add_element(mltest, aa);
	mouse_list_add_element(mltest, bb);
	mouse_list_add_element(mltest, cc);
	printf("WE %d \n", mltest->count);
	int lsone = mltest->head->val->lifespan;
	int lstwo = mltest->tail->val->lifespan;
	int lsthree = mltest->head->next->val->lifespan;
	printf("ARE %d \n", lsone);
  printf("HERE %d \n", lstwo);
  printf("AGAIN %d \n", lsthree);


  mouse* new1 = pop_mouse_left(mltest);
  int yolo = new1->lifespan;
  printf("TESTER1: %d \n", yolo);

  mouse* new2 = pop_mouse_left(mltest);
  int yolo2 = new2->lifespan;
  printf("TESTER2: %d \n", yolo2);

  mouse* new3 = pop_mouse_left(mltest);
  int yolo3 = new3->lifespan;
  printf("TESTER3: %d \n", yolo3);

  mouse* new4 = pop_mouse_left(mltest);
  if(new4 == NULL)
  {
  	printf("ITS NULL\n");
  }


	nest_list* nltest = nest_list_create();
	nest none = { .larva = 10};
	nest ntwo = { .larva = 20};
	nest nthree = { .larva = 30};
	nest* naa = &none;
	nest* nbb = &ntwo;
	nest* ncc = &nthree;
	nest_list_add_element(nltest, naa);
	nest_list_add_element(nltest, nbb);
	nest_list_add_element(nltest, ncc);
	printf("nWE %d \n", nltest->count);
	int nlsone = nltest->head->val->larva;
	int nlstwo = nltest->tail->val->larva;
	int nlsthree = nltest->head->next->val->larva;
	printf("nARE %d \n", nlsone);
  printf("nHERE %d \n", nlstwo);
  printf("nAGAIN %d \n", nlsthree);


  nest* nnew1 = pop_nest_left(nltest);
  int nyolo = nnew1->larva;
  printf("nTESTER1: %d \n", nyolo);

  nest* nnew2 = pop_nest_left(nltest);
  int nyolo2 = nnew2->larva;
  printf("nTESTER2: %d \n", nyolo2);

  nest* nnew3 = pop_nest_left(nltest);
  int nyolo3 = nnew3->larva;
  printf("nTESTER3: %d \n", nyolo3);

  nest* nnew4 = pop_nest_left(nltest);
  if(nnew4 == NULL)
  {
  	printf("nITS NULL\n");
    }
}

// void read_input_file(char* filepath, char* entity){
//   MPI_File fp;
//   MPI_File_open(MPI_COMM_WORLD, filepath, MPI_MODE_RDWR, MPI_INFO_NULL, &fp);
//   // numRowsPer, myRank
//   int file_offset = numRowsPer*myRank;
//   for(int i = 0; i < numRowsPer; i++){
//     MPI_File_read_at_all(&fp, );
//   }
//   char line[universeSize*4]; // make sure our line is big enough
//   MPI_File_close(&fp);
// }

// void parse_input_files(char* dirpath){
//   DIR *dir;
//   struct dirent *ent;
//   if ((dir = opendir (dirpath)) != NULL) {
//     while ((ent = readdir (dir)) != NULL) {
//       if(strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0){
//         char* underscore = strchr(ent->d_name, '_');
//         char* period = strchr(ent->d_name, '.');
//         int underscore_index = (int)(underscore - ent->d_name);
//         int period_index = (int)(period - ent->d_name);
//         if(underscore_index >= 0 && underscore_index < strlen(ent->d_name) && period_index > underscore_index){
//           char entity[period_index - underscore_index];
//           strncpy(entity, ent->d_name + underscore_index+1, period_index - underscore_index -1);
//           char filepath[strlen(dirpath)+strlen(ent->d_name)];
//           strncpy(filepath, dirpath, strlen(dirpath));
//           strncpy(filepath+strlen(dirpath), ent->d_name, strlen(ent->d_name));
//           read_input_file(filepath, entity);
//           //printf ("\tfilename[%s] entity filepath[%s]\n", ent->d_name, filepath);
//         } else{
//           perror("bad file name!");
//         }
		
//       }
//     }
//     closedir (dir);
//   } else{
//     perror("failed to read file 2");
//   }
// }

// void initUniverse() { 
//   	/*Comments: It's going to be hard to allocate a total number of mice across the entire
//   	universe, due to it being across ranks. I was thinking we create a Python program to do it, output 
//   	the allocations to a text file, and this program has each rank read its portion of allocations from the
//   	text file. */
//     // dir '/boards'
//     // subdir '/16x16'
//   char cwd[1024];
//   DIR *dir;
//   struct dirent *ent;
//   if (getcwd(cwd, sizeof(cwd)) != NULL) // get current working dir
//     fprintf(stdout, "Current working dir: %s\n", cwd);
//   else
//     perror("getcwd() error");
//   strncpy(cwd+strlen(cwd), "/boards/", 8);
//   printf("new dir: %s\n", cwd);
//   if ((dir = opendir (cwd)) != NULL) { // find the correct input file directory
//     while ((ent = readdir (dir)) != NULL) {
//       if(strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0){
//         char* x = strchr(ent->d_name, 'x');
//         int x_index = (int)(x - ent->d_name);
//         if(x_index >= 0 && x_index < strlen(ent->d_name)){
//           char dir_uni_size_str[x_index];
//           strncpy(dir_uni_size_str, ent->d_name, x_index);
//           int dir_uni_size = atoi(dir_uni_size_str);
//           if(dir_uni_size == universeSize){ // read in correct input files
//             char dirpath[strlen(cwd)+strlen(ent->d_name)+1];
//             strncpy(dirpath, cwd, strlen(cwd));
//             strncpy(dirpath+strlen(cwd), ent->d_name, strlen(ent->d_name));
//             dirpath[strlen(cwd)+strlen(ent->d_name)] = '/';
//             //printf ("dirname[%s], dirpath[%s]\n", ent->d_name, dirpath);
//             parse_input_files(dirpath);
//           }
		  
//         } else
//           printf("inproper universe dir[%s]\n", ent->d_name);
//       }
//     }
//     closedir (dir);
//   } else{
//     perror("failed to read file");
//   }

// }