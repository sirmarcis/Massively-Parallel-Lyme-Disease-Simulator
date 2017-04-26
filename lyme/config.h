

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
int mouseThreshold;
int rowLowerBound;
int rowUpperBound;
int prevRankID;
int nextRankID;
int numMicePerNest;
int mouseLifespan;
int uninfectedNymph;
int larvaSpawnDay;
int numLarva;



typedef struct configurationInfo {		//struct defining a configuration
	int days; 				//number of simulation days
	float biteThreshold; 		//infection rate
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
	int mouseThreshold;
	int rowLowerBound;
	int rowUpperBound;
	int prevRankID;
	int nextRankID;
	int numMicePerNest;
	int mouseLifespan;
	int uninfectedNymph;
	int larvaSpawnDay;
	int numLarva;	
} config;

void initConfigs();
void useConfiguration(int configNum);