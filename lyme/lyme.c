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
#include <mouse.h>

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

/***************************************************************************/
/* Thread Structs **********************************************************/
/***************************************************************************/

typedef struct threadInfo {		//struct defining the information for each thread
	int * myTID;				//given thread id
} thread;

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
	int numMice;				 //count for number of mice in cell
	int numDeer;				 //count for number of deer in cell
	mouse * miceinNest;			 //array of mice in nest
	deer * deerinNest;			 //array of deer in nest
} nest;


/***************************************************************************/
/* Global Variables ********************************************************/
/***************************************************************************/

/*
int days; 				//number of simulation days
int infectionRate; 		//infection rate
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

*/

nest ** universe;		//universe board
pthread_barrier_t barrier;	//barrier for threads

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
void initUniverse();
void initMouse(int i, int j, int trueRow, int numMicePerNest, int mouseLifespan);
void initTicks(int i, int j, int trueRow, int bandStart, int bandEnd, int uninfectedNymph);
void pthreadCreate();
void * updateUniverse();
void readCommandLineArgs(int argc, char* argv[]);



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
	

// Allocate my rank's chunk of universe
	numRowsPer = universeSize / numRanks;
  	

//Initialize universe with ticks, mice, and deer
	initUniverse();

// Start timing
	double start = 0;
	double end = 0;
	if (myRank == 0) start = MPI_Wtime();


	MPI_Barrier( MPI_COMM_WORLD );

// End timing
	if (myRank == 0)  {
		end = MPI_Wtime();
		printf("Time taken: %lf s\n", end - start);
	}

//Frees

	MPI_Barrier( MPI_COMM_WORLD );
	MPI_Finalize();
	return 0;
}

void readCommandLineArgs(int argc, char* argv[]){
	
	days = 1;
	infectionRate = 25;
	pthreads = 0;
	universeSize =  16;
	miceTravelDays = 4;
	tickFeedingDays = 3;
	totalNumMouse = 9;
	totalNumDeer = 2;
	carryLarva = 10;
	carryNymph = 5;
	carryAdult = 2;
	tickBand = 2;
	pthreads = 2;

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

void * updateUniverse() {
	return NULL;
}

void pthreadCreate() {

	//Initialize barrier
	pthread_barrier_init(&barrier, NULL, pthreads);

	//Create array holding total number of threads
	pthread_t tid[pthreads];
	int i=1;
	for (i=1; i < pthreads; i++) {
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

	//make main thread have tid = 0
	i=0;
	thread * t = (thread *) malloc(sizeof(thread));
	int * id = (int *) malloc(sizeof(int));
	*id = i;
	t->myTID = id;
	int rc = pthread_create( &tid[i], NULL, updateUniverse, t);
	if (rc != 0) {
		fprintf( stderr, "pthread_create() failed (%d): %s\n", rc, strerror( rc ));
		return;
	}


	//Wait for threads to join the main
	for (i=0; i < pthreads; i++) {
		unsigned int * x;
		pthread_join(tid[i], (void **) &x);
	}



}

//Initializes mice in a checkerboard fashion
void initMouse(int i, int j, int trueRow, int numMicePerNest, int mouseLifespan) {
	nest n;
	if (trueRow % 2 == 0) {
		if (j % 2 == 0) {
			//make Mice objects
			n.numMice = numMicePerNest;
			n.miceinNest = (mouse *) malloc(numMicePerNest * sizeof(mouse));
			for(int k = 0; k < numMicePerNest; k++){
				mouse m;
				m.lifespan = mouseLifespan;
				m.numDaysTraveled = 0;
				m.carrying = 0;
				m.typeTickCarrying = 0;
				m.infected = 0;
				n.miceinNest[k] = m;
			}
		}

		else {
			n.numMice = 0;
			n.miceinNest = NULL; // note: this may haveto change
		}
	}
	else {
		if (j % 2 == 1) {
			//make Mice objects
			n.numMice = numMicePerNest;
			n.miceinNest = (mouse *) malloc(numMicePerNest * sizeof(mouse));
			for(int k = 0; k < numMicePerNest; k++){
				mouse m;
				m.lifespan = mouseLifespan;
				m.numDaysTraveled = 0;
				m.carrying = 0;
				m.typeTickCarrying = 0;
				m.infected = 0;
				n.miceinNest[k] = m;
			}
		}

		else {
			n.numMice = 0;
			n.miceinNest = NULL; // note: this may haveto change
		}
	}
	universe[i][j] = n;
}

// Initializes nymphs within a column band, every fourth row
void initTicks(int i, int j, int trueRow, int bandStart, int bandEnd, int uninfectedNymph) {

	universe[i][j].larva = 0;
	universe[i][j].infectedNymph = 0;
	universe[i][j].uninfectedAdult = 0;
	universe[i][j].infectedAdult = 0;
	
	if (trueRow % 4 == 0 && j >=bandStart && j < bandEnd) { // then there are ticks in this cell
		if(trueRow % (universeSize/2) == 0 && trueRow != 0){ // then this tick nest has lyme disease
			universe[i][j].infectedNymph = uninfectedNymph/2;
			universe[i][j].uninfectedNymph = uninfectedNymph/2;
			printf("YAY infected ticks i[%d] j[%d]\n", trueRow,j);
		} else{
			universe[i][j].uninfectedNymph = uninfectedNymph;
		}
	} else
		universe[i][j].uninfectedNymph = 0;
}

// Initializes ticks and mice
void initUniverse() {
	int numMicePerNest = 5;
	int mouseLifespan = 15;
	int uninfectedNymph = 1000;
	int trueRow = myRank * numRowsPer;
	int bandStart = universeSize/2 - tickBand/2;
	int bandEnd = universeSize/2 + tickBand/2;
	int i=0;
	int j=0;
	universe = (nest **) malloc(numRowsPer * sizeof(nest *));

	for (i=0; i < numRowsPer; i++) {
		universe[i] = (nest *) malloc(universeSize * sizeof(nest));

		for (j=0; j < universeSize; j++) {
			initMouse(i, j, trueRow, numMicePerNest, mouseLifespan);
			initTicks(i, j, trueRow, bandStart, bandEnd, uninfectedNymph);
			
		}
	trueRow ++;
  }

}

/***************************************************************************/
/* Other Functions  ********************************************************/
/***************************************************************************/

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