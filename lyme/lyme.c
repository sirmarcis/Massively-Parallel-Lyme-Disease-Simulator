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
   	universeSize =  16;
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
   //initUniverse();

// Start timing
    double start = 0;
    double end = 0;
    if (myRank == 0) start = MPI_Wtime();


   MPI_Barrier( MPI_COMM_WORLD );

// End timing
   if (myRank == 0) end = MPI_Wtime();
   printf("Time taken: %lf s\n", end - start);

  //Frees

   MPI_Barrier( MPI_COMM_WORLD );
   MPI_Finalize();
   return 0;
}

/***************************************************************************/
/* Other Functions  ********************************************************/
/***************************************************************************/

void read_input_file(char* filepath, char* entity){
  MPI_File fp;
  MPI_File_open(MPI_COMM_WORLD, filepath, MPI_MODE_RDWR, MPI_INFO_NULL, &fp);
  // numRowsPer, myRank
  int file_offset = numRowsPer*myRank;
  for(int i = 0; i < numRowsPer; i++){
    MPI_File_read_at_all(&fp, );
  }
  char line[universeSize*4]; // make sure our line is big enough
  MPI_File_close(&fp);
}

void parse_input_files(char* dirpath){
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (dirpath)) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
      if(strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0){
        char* underscore = strchr(ent->d_name, '_');
        char* period = strchr(ent->d_name, '.');
        int underscore_index = (int)(underscore - ent->d_name);
        int period_index = (int)(period - ent->d_name);
        if(underscore_index >= 0 && underscore_index < strlen(ent->d_name) && period_index > underscore_index){
          char entity[period_index - underscore_index];
          strncpy(entity, ent->d_name + underscore_index+1, period_index - underscore_index -1);
          char filepath[strlen(dirpath)+strlen(ent->d_name)];
          strncpy(filepath, dirpath, strlen(dirpath));
          strncpy(filepath+strlen(dirpath), ent->d_name, strlen(ent->d_name));
          read_input_file(filepath, entity);
          //printf ("\tfilename[%s] entity filepath[%s]\n", ent->d_name, filepath);
        } else{
          perror("bad file name!");
        }
        
      }
    }
    closedir (dir);
  } else{
    perror("failed to read file 2");
  }
}

void initUniverse() { 
  	/*Comments: It's going to be hard to allocate a total number of mice across the entire
  	universe, due to it being across ranks. I was thinking we create a Python program to do it, output 
  	the allocations to a text file, and this program has each rank read its portion of allocations from the
  	text file. */
    // dir '/boards'
    // subdir '/16x16'
  char cwd[1024];
  DIR *dir;
  struct dirent *ent;
  if (getcwd(cwd, sizeof(cwd)) != NULL) // get current working dir
    fprintf(stdout, "Current working dir: %s\n", cwd);
  else
    perror("getcwd() error");
  strncpy(cwd+strlen(cwd), "/boards/", 8);
  printf("new dir: %s\n", cwd);
  if ((dir = opendir (cwd)) != NULL) { // find the correct input file directory
    while ((ent = readdir (dir)) != NULL) {
      if(strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0){
        char* x = strchr(ent->d_name, 'x');
        int x_index = (int)(x - ent->d_name);
        if(x_index >= 0 && x_index < strlen(ent->d_name)){
          char dir_uni_size_str[x_index];
          strncpy(dir_uni_size_str, ent->d_name, x_index);
          int dir_uni_size = atoi(dir_uni_size_str);
          if(dir_uni_size == universeSize){ // read in correct input files
            char dirpath[strlen(cwd)+strlen(ent->d_name)+1];
            strncpy(dirpath, cwd, strlen(cwd));
            strncpy(dirpath+strlen(cwd), ent->d_name, strlen(ent->d_name));
            dirpath[strlen(cwd)+strlen(ent->d_name)] = '/';
            //printf ("dirname[%s], dirpath[%s]\n", ent->d_name, dirpath);
            parse_input_files(dirpath);
          }
          
        } else
          printf("inproper universe dir[%s]\n", ent->d_name);
      }
    }
    closedir (dir);
  } else{
    perror("failed to read file");
  }

}