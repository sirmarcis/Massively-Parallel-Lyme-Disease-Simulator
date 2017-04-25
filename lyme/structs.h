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
	mouse_list * miceInNest;	 //array of mice in nest
	deer * deerInNest;			 //array of deer in nest
	pthread_mutex_t mutex;

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
	int mustMove;					// if 1, this mouse must move, else if 0, mouse can stay
	int nextHome_x;				// next x location of this mouse with respect to the entire universe
	int nextHome_y;				// next y location of this mouse with respect to the entire universe

	int direction_x;				// can be any value between 0 and 8, inclusive.  0 is straight up, subsiquent values go clockwise
	int direction_y;
	int mouseUID; 				// unique ID of a mouse within a rank (not universe)
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