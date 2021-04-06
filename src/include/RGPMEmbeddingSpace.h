#ifndef RGPMEMBEDDINGSPACE_H
#define RGPMEMBEDDINGSPACE_H

#include "logging.h"
#include "constants.h"
#include "utils.h"
#include "graph.h"
#include "random.h"
#include "canonical.h"
#include "VertexInducedEmbedding.h"
#include "SuperEmbedding.h"
#include "EmbeddingUtils.h"
#include "AggregatorPatternCounter.h"
#include "BasicEmbeddingSpace.h"
#include "TourStats.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <list>
#include <unordered_set>
#include <tbb/task_arena.h>
#include <unordered_map>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <graph.hh>
#include <cache.h>
#include <fstream>


template <class T, class A> 
class EmbeddingSpaceRGPM : public EmbeddingSpace<T,A> {
		
	public : 
		int snSize; // embedding space related size
		int nthreads;
	
		std::vector<TourStatsWords> tourStatsThreads;
		ModSet mods_empty;

		EmbeddingSpaceRGPM(int , Graph *); 
		EmbeddingSpaceRGPM(int , int, Graph *); 
		EmbeddingSpaceRGPM(int , int, Graph *, int); 
		EmbeddingSpaceRGPM(int , int, Graph *, std::string, std::string); 
		EmbeddingSpaceRGPM(int , int, Graph *, std::string, std::string, int); 
		void checkConfigFileRGPM();
		
		void run_rw();
		virtual TourStatsWords getGroupStats(T &, SuperEmbedding<T> &, int);
		virtual TourStatsWords &groupEstimateUsingTour(T, SuperEmbedding<T>&, int, TourStatsWords&);
		virtual TourStatsWords groupEstimateAllTours(T&, SuperEmbedding<T>&, int);

		SuperEmbedding<T> createBFSGroup(T &, int);

		std::pair<T,bool> getRandomNeighborSN(SuperEmbedding<T> &);
		ModSet getModificationsToSN(T &, SuperEmbedding<T> &);
		ModSet getModificationsOutSN(T &, SuperEmbedding<T> &);

		uint MAX_NUM_TOURS=10000;      
		uint MAX_NUM_TOURS_BATCH=10000;      
		int MAX_TOUR_STEPS=100000;

};


#endif

