#ifndef _STRUCTSH_
#define _STRUCTSH_

#include <pthread.h>
struct nestInfo;
struct mouseInfo;
struct deerInfo;
struct threadInfo;
struct mouseListInfo;
struct nestListInfo;

typedef struct nestInfo nest;
typedef struct mouseInfo mouse;
typedef struct deerInfo deer;
typedef struct threadInfo thread;
typedef struct mouseListInfo mouse_list;
typedef struct nestListInfo nest_list;
typedef struct mouseNode mouseNode;
typedef struct nestNode nestNode;

struct nestInfo {		//struct for each cell in board
	int larva;					//counts of each type of tick
	int uninfectedNymph;
	int infectedNymph;
	int uninfectedAdult;
	int infectedAdult;
	int numMice;				 //count for number of mice in cell
	int numDeer;				 //count for number of deer in cell
	mouse * miceinNest;	 //array of mice in nest
	deer * deerinNest;			 //array of deer in nest
	//nest * prev;
	//nest * next;
};

struct mouseInfo {
	int lifespan;				//lifespan of mouse
	int numDaysTraveled;		//number of days mouse has traveled
	int carrying;				//boolean if mice is carrying ticks
	int typeTickCarrying;
	int infected;
	//mouse * prev;
	//mouse * next;
	int tickDropOffDate;
	nest * currentNest;
	int nextHome_x;				// next x location of this mouse with respect to the entire universe
	int nextHome_y;				// next y location of this mouse with respect to the entire universe
};

struct deerInfo {
	int infected;
	int carrying;
};

struct threadInfo {		//struct defining the information for each thread
	int * myTID;				//given thread id
};


struct mouseListInfo {
	int count;
	mouseNode *head;
	mouseNode *tail;
	pthread_mutex_t mutex;
};

struct nestListInfo {
	int count;
	nestNode *head;
	nestNode *tail;
	pthread_mutex_t mutex;
};

struct mouseNode {
	mouseNode * prev;
	mouseNode * next;
	mouse * val;
};


struct nestNode {
	nestNode * prev;
	nestNode * next;
	nest * val;
};



#endif