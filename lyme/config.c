
#include <config.h>

config allConfigurations[2];
int numConfigurations = 2;

void initConfigs(){
	// INIT ALL CONFIGURATIONS HERE
	config config0; // configuration 0 (current default)
	config config1;

	// INIT config0
	config0.days = 1;
	config0.biteThreshold = .25;
	config0.universeSize =  16;
	config0.miceTravelDays = 4;
	config0.tickFeedingDays = 3;
	config0.totalNumMouse = 9;
	config0.totalNumDeer = 2;
	config0.carryLarva = 10;
	config0.carryNymph = 5;
	config0.carryAdult = 2;
	config0.tickBand = 2;
	config0.pthreads = 2;
	config0.mouseThreshold = 1;
	config0.numRowsPer = universeSize / numRanks;
	config0.rowLowerBound = numRowsPer * myRank;
	config0.rowUpperBound = numRowsPer * (myRank+1);
	config0.numMicePerNest = 5;
	config0.mouseLifespan = 100;
	config0.uninfectedNymph = 1000;
	config0.larvaSpawnDay = 90;
	config0.numLarva = 1000;

	allConfigurations[0] = config0;

	// INIT config1
	config1.days = 1;
	config1.biteThreshold = .25;
	config1.universeSize =  16;
	config1.miceTravelDays = 4;
	config1.tickFeedingDays = 3;
	config1.totalNumMouse = 9;
	config1.totalNumDeer = 2;
	config1.carryLarva = 10;
	config1.carryNymph = 5;
	config1.carryAdult = 2;
	config1.tickBand = 2;
	config1.pthreads = 2;
	config1.mouseThreshold = 1;
	config1.numRowsPer = universeSize / numRanks;
	config1.rowLowerBound = numRowsPer * myRank;
	config1.rowUpperBound = numRowsPer * (myRank+1);
	config1.numMicePerNest = 5;
	config1.mouseLifespan = 100;
	config1.uninfectedNymph = 1000;
	config1.larvaSpawnDay = 90;
	config1.numLarva = 1000;

	allConfigurations[1] = config1;
}

void useConfiguration(int configNum){
	if(configNum < numConfigurations){
		config currConfig = allConfigurations[configNum];
		days = currConfig.days;
		biteThreshold = currConfig.biteThreshold;
		universeSize = currConfig.universeSize;
		numRowsPer = currConfig.numRowsPer;
		miceTravelDays = currConfig.miceTravelDays;
		tickFeedingDays = currConfig.tickFeedingDays;
		totalNumMouse = currConfig.totalNumMouse;
		totalNumDeer = currConfig.totalNumDeer;
		carryLarva = currConfig.carryLarva;
		carryNymph = currConfig.carryNymph;
		carryAdult = currConfig.carryAdult;
		tickBand = currConfig.tickBand;
		pthreads = currConfig.pthreads;
		mouseThreshold = currConfig.mouseThreshold;
		rowLowerBound = currConfig.rowLowerBound;
		rowUpperBound = currConfig.rowUpperBound;
		prevRankID = currConfig.prevRankID;
		nextRankID = currConfig.nextRankID;
		numMicePerNest = currConfig.numMicePerNest;
		mouseLifespan = currConfig.mouseLifespan;
		uninfectedNymph = currConfig.uninfectedNymph;
		larvaSpawnDay = currConfig.larvaSpawnDay;
		numLarva = currConfig.numLarva;
	}
}