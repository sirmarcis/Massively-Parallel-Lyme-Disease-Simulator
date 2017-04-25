#ifndef _MOUSEH_
#define _MOUSEH_

typedef struct mouseInfo {
	int lifespan;				//lifespan of mouse
	int numDaysTraveled;		//number of days mouse has traveled
	int carrying;				//boolean if mice is carrying ticks
	int typeTickCarrying;
	int infected;
  struct mouseInfo * prev;
  struct mouseInfo * next;

} mouse;

#endif