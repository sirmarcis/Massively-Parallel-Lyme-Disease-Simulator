#ifndef _STRUCTSH_
#define _STRUCTSH_

#include <pthread.h>

typedef struct mouseInfo {
	int lifespan;				//lifespan of mouse
	int numDaysTraveled;		//number of days mouse has traveled
	int carrying;				//boolean if mice is carrying ticks
	int typeTickCarrying;
	int infected;
	struct mouseInfo * prev;
	struct mouseInfo * next;

} mouse;

typedef struct deerInfo {
	int infected;
	int carrying;
} deer;

typedef struct threadInfo {		//struct defining the information for each thread
	int * myTID;				//given thread id
} thread;

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
	struct nestInfo * prev;
	struct nestInfo * next;
} nest;


typedef struct {
	int count;
	mouse *head;
	mouse *tail;
	pthread_mutex_t mutex;
} mouse_list;

typedef struct {
	int count;
	nest *head;
	nest *tail;
	pthread_mutex_t mutex;
} nest_list;



#endif