#ifndef MATRIOSKAEMBEDDINGSPACE_H
#define MATRIOSKAEMBEDDINGSPACE_H

#include "RGPMEmbeddingSpace.h"
#include "reservoir.h"
#include "TourStats.h"
#include <iostream>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/task_group.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/task_arena.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>
#include <array>
#include <chrono>
#include <sstream>      // std::stringstream
#include <thread>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <gmp.h>
#include <boost/functional/hash.hpp>

typedef reservoir<earray<int>> ReservoirWords;

template <class T, class A>
class EmbeddingSpaceMSK:public EmbeddingSpaceRGPM<T,A> {
	
	public:
	std::vector<std::unordered_set<int>> layers;
	std::vector<uint> layerMap; // node to layer
	std::vector< std::vector<std::pair<uint,uint> > > nodeLayerIdx; // node to layer
	std::vector<uint> layerNumPartitions; 
	std::vector<float128> partitionNumEmbs; 
	std::vector<float128> partitionDegree; 
	std::unordered_map<uint, std::pair<uint, uint>> partition2layerMap;
	std::unordered_map<std::pair<uint, uint>, uint, boost::hash<std::pair<uint, uint>>> layer2partitionMap;
	std::vector<uint> initialPartitionMap;
	std::unordered_set<int> initLayer;
	//tbb::concurrent_hash_map<earray<uint>, float128, earrayhash<uint>> total_counts_estimate;

	uint numInitialSuperNodePartitions;
	
	//these vectors use a matrix represetation (numPartitions x numPartitions)
	//std::vector<ReservoirWords> rs;
	//typename std::unordered_map<size_t, std::pair<float128,ReservoirWords>> partitionRelationMap; // it keeps the degree beteween two partitions
	std::vector<std::pair<float128,ReservoirWords>> partitionRelationMap;

	thread_local static std::unordered_map<uint, int> initialPartitionCounter; 

	double INIT_SUPERNODE_MAX_SCORE = 1000;
	double INIT_SUPERNODE_MIN_SCORE = 1;
	int NUM_INITIAL_PARTITIONS = 1000000;
	int INITIAL_PARTITION_SIZE = 10;
	int MAX_RESERVOIR_SIZE=10;
	int PRE_RESERVOIR_TYPE=-1;
	int TOURS_DIST_TYPE=-1;
        bool FORCE_PARTITION_RETURN=false;
        bool RANDOMIZE_SCORES=false;
	bool PARTITION_RETURN_REJECTION=true;
	uint128_t REJECTION_COUNTER3 = 0;
	uint128_t REJECTION_COUNTER4 = 0;
	int PARTITION_REDUCTION_FACTOR=1;
	double PARTITION_ERROR_BOUND=0.01;

	EmbeddingSpaceMSK(int , Graph *); 
	EmbeddingSpaceMSK(int , Graph *, std::string, std::string);
	EmbeddingSpaceMSK(int , Graph *, std::string, std::string, int);

	void fill_reservoir_exact_rec(T &, int);
	void fill_reservoir_exact();
	void fill_reservoir_rw();
	void run_exact_rec(T &, int);
	void run_exact();
	void run_rw();
	TourStatsWords &groupEstimateUsingTour(uint, float128,  uint, int, TourStatsWords&);
	TourStatsWords groupEstimateAllTours(uint, float128,  uint, int);
	TourStatsWords &groupEstimateUsingTourReservoir(uint, float128,  uint, int, TourStatsWords&);
	TourStatsWords groupEstimateAllToursReservoir(uint, float128,  uint, int);
	void groupEstimateUsingBruteForceUtil(T&, int, uint, std::unordered_set<int> &, TourStatsWords &);
	TourStatsWords groupEstimateUsingBruteForce(uint, uint);
	void groupEstimateUsingBruteForceUtilReservoir(T&, int, uint, std::unordered_set<int> &, TourStatsWords &);
	TourStatsWords groupEstimateUsingBruteForceReservoir(uint, uint);
	TourStatsWords partitionEstimateAllTours(uint, uint, SuperEmbedding<T> &, int);
	TourStatsWords partitionEstimateUsingTour(uint, uint numPartitions, SuperEmbedding<T> &, int, int);
	bool isInPreviousLevels(uint, T &e);

	void initAllLevels(T &, Graph *);
	void initPartitionLayers(T &, Graph *);
	void getInitialLayer(Graph *g, int maxNumRegions, int maxRegionSize, std::vector<int> &scores);
	std::vector<std::unordered_set<int>> buildInitialPartitions(boost::dynamic_bitset<> &, boost::dynamic_bitset<> &, Graph *, int, int, int, int, std::vector<int>&);
	std::vector<std::unordered_set<int>> buildAllInitialPartitionsBFS(boost::dynamic_bitset<> &, boost::dynamic_bitset<> &, Graph *, int, int, int, int, std::vector<int>&);
	void getSubPartitionsUtil(T &, std::unordered_set<int> &, std::set<std::set<int>> &, Graph *, int, int );
	void getSubPartitions(Graph *g, std::map<int, std::set<std::set<int> > > &, int);

	uint hashEmbedding (T &e, std::vector<uint> &hf, uint);
	std::vector<uint> create_hf(int k);
	void computeLayersBFS(T &e, Graph *);

	double getNodeCost(boost::dynamic_bitset<> &, int, int);
	T getRandomEmbeddingInLevel(uint);
	int countNodesInPartition(uint partition_idx);
	void getPartitionInfo(Graph *);	
	void checkPartitionConnection();
	std::unordered_set<int> getLargestConnectedPartition(std::unordered_set<int> &, boost::dynamic_bitset<> &, Graph *, int, uint);
	std::unordered_set<int> getLargestConnectedPartitionFromNode(int, std::unordered_set<int> &, boost::dynamic_bitset<> &, Graph *, int);

	inline uint hashEmbedding (std::vector<int> &words, std::vector<uint> &hf, uint numSubpart) {
		/*uint maxL = 0;
		  uint i = 0;
		  while (i<words.size()){
		//val +=   (long unsigned int) words[i] * (long unsigned int) hf[i];
		if (layerMap[words[i]] < maxL) {
		maxL = layerMap[words[i]];
		val=0;
		}
		if (layerMap[words[i]] == maxL) val++;

		i++;
		}
		if (maxL == 0) return 0;
		else return (val-1);
		*/
		return 0;
	}

	inline uint getPartitionId(T &e, Mod mod) {
		int pid=0;
		earray<int> &words = e.getWords();
		earray<uint> initialPids;
		for (int i = 0; i < e.getNumWords(); i++) {
			int nodeId = 0;
			if (mod.rmId==words[i]) nodeId = mod.addId;
			else nodeId = words[i];

			pid+=layerMap[nodeId];
			
			if (layerMap[nodeId]==0) initialPids[i] = initialPartitionMap[nodeId];
			else initialPids[i] = std::numeric_limits<uint>::max(); 
		}

		std::sort(initialPids.begin(), initialPids.begin()+e.getNumWords());

		int i = 0; 
		int current = 0;
		int largest = 0;
	        while (i < e.getNumWords() && initialPids[i]!=std::numeric_limits<uint>::max()) {
			current++;
			if (initialPids[i]!=initialPids[i+1]) {
				if (largest < current) largest = current;
				current = 0;
			}
			//std::cout << "initial partition: " << initialPids[i] << std::endl;
			i++;
		}
		if (largest==0) largest = current;
		int adjust = i - largest;
		/*std::cout << e << " adjust " << adjust << " " << i << " " << largest<< std::endl;
		if (i == 0 && largest != 0)  {
			exit(1);
		}*/
		//return std::ceil((double)(pid)/PARTITION_REDUCTION_FACTOR);
		return std::ceil((double)(pid+adjust)/PARTITION_REDUCTION_FACTOR);
	}

	inline uint getPartitionId(T &e) {
		int pid=0;
		earray<int> &words = e.getWords();
		earray<uint> initialPids;
		for (int i = 0; i < e.getNumWords(); i++) {
			pid+=layerMap[words[i]];
			
			if (layerMap[words[i]]==0) initialPids[i] = initialPartitionMap[words[i]];
			else initialPids[i] = std::numeric_limits<uint>::max(); 
		}

		std::sort(initialPids.begin(), initialPids.begin()+e.getNumWords());

		int i = 0; 
		int current = 0;
		int largest = 0;
	        while (i < e.getNumWords() && initialPids[i]!=std::numeric_limits<uint>::max()) {
			current++;
			if (initialPids[i]!=initialPids[i+1]) {
				if (largest < current) largest = current;
				current = 0;
			}
			//std::cout << "initial partition: " << initialPids[i] << std::endl;
			i++;
		}
		if (largest==0) largest = current;
		int adjust = i - largest;

		/*std::cout << e << " adjust " << adjust << " " << i << " " << largest<< std::endl;
		if (i > 0 && largest == 0)  {
			exit(1);
		}*/
		return std::ceil((double)(pid+adjust)/PARTITION_REDUCTION_FACTOR);
		//return std::ceil((double)(pid)/PARTITION_REDUCTION_FACTOR);
	}

	inline size_t getMatrixPositionInArray(int i, int j) {

		int n = (int) partition2layerMap.size();
		//return (c*(c-1))/2 + r;
		/*size_t seed=0;
		  boost::hash_combine(seed,  r * 15487469);
		  boost::hash_combine(seed,  c * 2654435761);
		  return seed;*/

		//return 0.5*(r+c)*(r+c+1)+c;
		if (n == 0 || i >= j) {
			std::cout << "error: reservoir index does not exist! " << " n: " << n << " i: " << i << " j: " << j  << std::endl;
			exit(1);
		}

		return (n*(n-1)/2) - (n-i)*((n-i)-1)/2 + j - i - 1;
	}

	void updatePartitionRelationReservoir(size_t idx, T& e) {
		partitionRelationMap[idx].second.insert(e.getWords());
	}
	void setDegreePartitionRelation(size_t idx, float128 d) {
		partitionRelationMap[idx].first = d;
	}
	float128 getDegreePartitionRelation(size_t idx) {
		return partitionRelationMap[idx].first;
	}
	uint getReservoirPartitionRelationSize(size_t idx) {
		return std::min((uint)partitionRelationMap[idx].second.total, partitionRelationMap[idx].second.cpty);
	}
	uint getReservoirPartitionTotalSize(size_t idx) {
		return partitionRelationMap[idx].second.total;
	}
	earray<int> &getReservoirPartitionRelationRandomElement(size_t idx, bool bias) {
		return partitionRelationMap[idx].second.random();
	}
	void resetPartitionRelation(size_t idx) {
		partitionRelationMap[idx].first = 0;
		partitionRelationMap[idx].second.reset();
	}


	/*void updatePatternCount(earray<uint> *pcode, float128 count) {
	  tbb::concurrent_hash_map<earray<uint>, float128, earrayhash<uint>>::accessor a;

	//earray<uint> key = *pcode;
	bool r = total_counts_estimate.insert(a, *pcode);       // creates by default if not exists, acquires lock

	if (r) { // it means that it is new
	a->second = count;
	}
	else {
	a->second += count;
	}

	a.release();

	}*/

	inline int getNumberOfNeighborsWithInitialPid(int wordId, uint initialNode) {
		if (nodeLayerIdx[wordId][0].second==0) return 0;
		std::vector<int> &neigh = this->g->getNeighborhoodIdxVertexOfVertexAt(wordId);
		
		//std::cout << "neighhood of node: " << wordId << " looking for initialPid: " << initialPartitionMap[initialNode];
		//for (int i : neigh)  std::cout << " [" << i << "," << layerMap[i] << "," << initialPartitionMap[i] << "] "; 
		//std::cout << std::endl;
		
		std::pair<std::vector<int>::iterator,std::vector<int>::iterator> bounds;
		bounds = std::equal_range(neigh.begin()+nodeLayerIdx[wordId][0].first, neigh.begin()+nodeLayerIdx[wordId][0].first+nodeLayerIdx[wordId][0].second, initialNode, VectorCompWithOthers(layerMap, initialPartitionMap));

		//std::cout << "low: " << bounds.first-neigh.begin() << " up: " << bounds.second-neigh.begin() << std::endl;
		return bounds.second-bounds.first;
	}

	inline std::pair<int, int> getRandomNeighborsWithInitialPid(int wordId, uint initialNode) {
		if (nodeLayerIdx[wordId][0].second==0) return std::pair<int, int> (0,0);
		std::vector<int> &neigh = this->g->getNeighborhoodIdxVertexOfVertexAt(wordId);

		std::pair<std::vector<int>::iterator,std::vector<int>::iterator> bounds;
		bounds = std::equal_range(neigh.begin()+nodeLayerIdx[wordId][0].first, neigh.begin()+nodeLayerIdx[wordId][0].first+nodeLayerIdx[wordId][0].second, initialNode, VectorCompWithOthers(layerMap, initialPartitionMap));

		if (bounds.first==bounds.second) return std::pair<int, int> (0,0);
		else return std::pair<int, int> (*Randness::getInstance()->random_element(bounds.first, bounds.second), bounds.second-bounds.first);
	}
	
	std::pair<Mod,bool> getNextRandomModificationByRejectionToSN(T &e, uint pid) {
		//std::cout << "getNextRandomModificationByRejection" << e << std::endl;
		std::pair<Mod, bool> mod;
		mod.second = false;

		earray<int> &words = e.getWords();
		std::unordered_map<uint, std::pair<int, int>> initialPids;
		int totalWeight=0;
		int largestInitPartition = 0;
		std::vector<int> layersCounts (layers.size(), 0);
		for (int i = 0; i < e.getNumWords(); i++) {
			totalWeight += this->g->getDegreeOfNodeAt(words[i]);
			for (uint j = 0; j < layers.size(); j++) layersCounts[j]+=nodeLayerIdx[words[i]][j].second;

			//std::cout << "node: " << words[i] << " layer id: " << layerMap[words[i]] << std::endl;
			if (layerMap[words[i]]==0) {
				//std::cout << "node from seed found: " << words[i] << " seed: " << initialPartitionMap[words[i]] << std::endl;
				std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.find(initialPartitionMap[words[i]]);
				if (it==initialPids.end()) {
					initialPids.insert(std::pair<uint, std::pair<int,int>> (initialPartitionMap[words[i]],std::pair<int,int>(words[i],1)));
					if (largestInitPartition==0) largestInitPartition=1;
				}
				else {
					it->second.second++;
					if (largestInitPartition<it->second.second) largestInitPartition=it->second.second;
				}
			}
		}

		/*for (std::unordered_map<uint, int>::iterator it = initialPids.begin(); it!= initialPids.end(); it++){
			std::cout << "initial seed: " << it->first << " count: " << it->second << std::endl;
		}*/

		for (std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.begin(); it!= initialPids.end();){
			if (it->second.second != largestInitPartition) {
				it = initialPids.erase(it);
			}
			else {
				++it;
			}
		}
		

		uint layersCountZero = 0;
		if (initialPids.empty()) {
			//std::cout << "majority initial seed: there is no majority seed!" << std::endl;
			for (int i = 0; i < e.getNumWords(); i++) layersCountZero+=nodeLayerIdx[words[i]][0].second;
		}
		else {
			//for (std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.begin(); it!= initialPids.end(); it++){
			//	std::cout << "majority initial seed: " << it->first << " count: " << it->second.second << std::endl;
			//}
			for (int i = 0; i < e.getNumWords(); i++) {
				//std::cout << "node: " << words[i] << " layerId: " << layerMap[words[i]] << " neighbors with layerId 0: " << nodeLayerIdx[words[i]][0].second << std::endl;
				for (std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.begin(); it!= initialPids.end(); it++) {
					layersCountZero+=getNumberOfNeighborsWithInitialPid(words[i], it->second.first);
				}
			}
		}
		//std::cout << " layersCountZero: " << layersCountZero << std::endl;
		uint currPid = getPartitionId(e);
		//std::cout << " pid: " << pid << " currPid: " << currPid << std::endl;

		uint articulation = EmbeddingUtils::articulation(e);
		

		int att = 0;
		do {
			att++;

			//select node to be removed
			int leaveIdx=0;
			int totalWeightPartition=0;
			uint layerId = 0;
			for (int i = 0; i < e.getNumWords(); i++) {
				if ((layerMap[words[i]]>0 && layerMap[words[i]]<currPid-pid)) continue;
				if ((layerMap[words[i]]==0 && pid+1<currPid)) continue;
				if ((layerMap[words[i]]==0 && initialPids.size()==1 && initialPids.find(initialPartitionMap[words[i]])!=initialPids.end())) continue;
				
				uint possibleLayerId = (layerMap[words[i]]==0) ? pid + 1 - currPid : pid + layerMap[words[i]] - currPid;
				if ((possibleLayerId!=1 && layersCounts[possibleLayerId]==0) || (possibleLayerId==1 && layersCounts[0]==0 && layersCounts[1]==0)) continue;
				//if ((possibleLayerId>1 && layersCounts[possibleLayerId]==0) || (possibleLayerId==0 && layersCountZero==0) || (possibleLayerId==1 && layersCounts[0]==0 && layersCounts[1]==0)) continue;
				int weight = 0;
				if (possibleLayerId==0 && !initialPids.empty()) {
					weight=layersCountZero;
					for (std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.begin(); it!= initialPids.end(); it++) {
						weight-=getNumberOfNeighborsWithInitialPid(words[i], it->second.first);
					}
				}
				else {
					weight = layersCounts[possibleLayerId]-nodeLayerIdx[words[i]][possibleLayerId].second;
					if (possibleLayerId==1) weight+=(layersCounts[0]-nodeLayerIdx[words[i]][0].second);
				}
				
				//std::cout << "removing node: " << words[i] << " weight: " << weight << " possibleLayerId: " << possibleLayerId << std::endl;
				double prob = Randness::getInstance()->random_uni01();
				if (weight >0 && prob <= (double) weight/(totalWeightPartition+weight)) {
					leaveIdx=i;
					layerId = possibleLayerId;
				}
				totalWeightPartition+=weight;
			}
			if (totalWeightPartition==0) {
				std::cout << "error: impossible to select a node to be removed. " << std::endl;
				exit(1);
			}

			mod.first.rmId = words[leaveIdx];
			//std::cout << "currPid: " << currPid << " pid: " << pid << " leaveIdx: " << leaveIdx << " layerMap: " << layerMap[words[leaveIdx]] << " layerId: " << layerId << std::endl;

			//select the anchor
			totalWeightPartition=0;
			uint anchorIdx=0;
			for (int i = 0; i < e.getNumWords(); i++) {
				if (i==leaveIdx) continue;
				//std::cout << "nodeLayerIdx of node: " << words[i] << ": " << nodeLayerIdx[words[i]][layerId].first << " " << nodeLayerIdx[words[i]][layerId].second << std::endl;
				int weight = 0;
				if (layerId==0 && !initialPids.empty()) {
					for (std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.begin(); it!= initialPids.end(); it++) {
						weight+=getNumberOfNeighborsWithInitialPid(words[i], it->second.first);
					}
				}
				else {
					weight = nodeLayerIdx[words[i]][layerId].second;
					if (layerId == 1) weight+=nodeLayerIdx[words[i]][0].second;
				}

				double prob = Randness::getInstance()->random_uni01();
				if (weight >0 && prob <= (double) weight/(totalWeightPartition+weight)) {
					anchorIdx=i;
				}
				totalWeightPartition+=weight;
			}
			if (totalWeightPartition==0) {
				std::cout << "error: impossible to select a node to be removed. " << std::endl;
				exit(1);
			}
				
			//select a node to get in
			if (layerId==0 && !initialPids.empty()) {
				totalWeightPartition=0;
				for (std::unordered_map<uint, std::pair<int,int>>::iterator it = initialPids.begin(); it!= initialPids.end(); it++) {
					std::pair<int, int> p = getRandomNeighborsWithInitialPid(words[anchorIdx], it->second.first);
					int weight = p.second;
					double prob = Randness::getInstance()->random_uni01();
					if (prob <= (double) weight/(totalWeightPartition+weight)) mod.first.addId=p.first;
					totalWeightPartition+=weight;
				}
				
			}
			else {
				uint idx = nodeLayerIdx[words[anchorIdx]][layerId].first;
				uint offset = nodeLayerIdx[words[anchorIdx]][layerId].second;
				if (layerId == 1) {
					idx = nodeLayerIdx[words[anchorIdx]][0].first;
					offset += nodeLayerIdx[words[anchorIdx]][0].second;
				}

				if (offset == 0) {
					std::cout << "currPid: " << currPid << " pid: " << pid << " anchorIdx: " << anchorIdx << " idx: " << idx << " offset: " << offset << std::endl;
	                                std::cout << "error: anchorIdx offset has not the expected value!" << std::endl;
					exit(1);
				}

				mod.first.addId = this->g->getRandomNodeNeighbor(words[anchorIdx], idx, offset);
			}
			//std::cout << "currPid: " << currPid << " pid: " << pid << " addId: " << mod.first.addId << std::endl;
			if ((layerId != 1 && layerMap[mod.first.addId]!=layerId) || (layerId == 1 && layerMap[mod.first.addId]!=0 && layerMap[mod.first.addId]!=1)) {
				std::cout << "error: addId has not the expected value!" << std::endl;
				exit(1);
			}

			if (e.hasWord(mod.first.addId)) continue;
			
			if (initialPids.empty()) {
				if (layerId == 1 && layerMap[mod.first.addId]==0) continue;
			}
			else if (initialPids.size()==1) {
				if (layerId == 0 && initialPids.find(initialPartitionMap[mod.first.addId])==initialPids.end()) continue;
				if (layerId == 1 && layerMap[mod.first.addId]==0 && initialPids.find(initialPartitionMap[mod.first.addId])!=initialPids.end()) continue;
			}
			else {
				if (layerId == 0 && initialPids.find(initialPartitionMap[mod.first.addId])==initialPids.end()) continue;
				if (layerId == 0 && layerMap[mod.first.rmId]==0 && initialPartitionMap[mod.first.addId]==initialPartitionMap[mod.first.rmId]) continue;
				if (layerId == 1 && layerMap[mod.first.addId]==0 && initialPids.find(initialPartitionMap[mod.first.addId])!=initialPids.end()) continue;
			}	

			int bias = 0;
			for (int i = 0; i < e.getNumWords(); i++)
				if (i!=leaveIdx && this->g->isNeighbor(words[i], mod.first.addId)) bias++;
			
			double prob = Randness::getInstance()->random_uni01();
			//std::cout << "currPid: " << currPid << " pid: " << pid << " addId: " << mod.first.addId << std::endl;
			if (prob<=1./(double)bias) {
				if (!isKthBitSet(articulation,leaveIdx)) {
					mod.second = true;
				}
				else {
					REJECTION_COUNTER4++;
					uint articulation2 = EmbeddingUtils::articulation(e, mod.first.addId);
					if (!isKthBitSet(articulation2,leaveIdx)) mod.second = true;
				}
			}
		} while (!mod.second && att < this->MAX_INIT_ATTEMPT);
		REJECTION_COUNTER3+=att;
		if (!mod.second) {
			std::cout << "warning: getNextRandomModificationByRejectionToSN fail. " << e << std::endl;
			exit(1);
		}
		return mod;
	}

	std::pair<Mod,bool> getNextRandomModificationToSN(T &e, uint pid) {
		Mod mod;
		std::unordered_map<size_t, std::unordered_set<int>> contractionMap;
		//std::cout << "getting neighbor for " << e << std::endl;

		std::unordered_set<int> expansions = e.getValidElementsForExpansion();
		for (std::unordered_set<int>::iterator it = expansions.begin(); it!= expansions.end(); it++) {
			std::pair<size_t,int> connectionHash = e.getWordConnectionHash(*it);
			std::unordered_map<size_t, std::unordered_set<int>>::iterator itContraction = contractionMap.find(connectionHash.first);
			if (itContraction == contractionMap.end()) {
				contractionMap.insert(std::make_pair(connectionHash.first, e.getValidElementsForContractionWithWord(*it)));
			}

			//checking if there is an error.
			itContraction = contractionMap.find(connectionHash.first);
			if (itContraction->second.empty()) {
				std::cout << "error: contractions empty!" << std::endl;
				exit(1);
			}

			for (std::unordered_set<int>::iterator it2 = itContraction->second.begin(); it2 != itContraction->second.end(); it2++) {
				if (getPartitionId(e, Mod(*it2, *it))!=pid) continue;

				double weight = 1.;
				double prob = Randness::getInstance()->random_uni01();
				if (prob <= (double) weight/(mod.totalWeight+weight)) {
					mod.rmId = *it2;
					mod.addId = *it;
				}
				mod.totalWeight++;
			}
		}

		//checking if there is an error.
		if (mod.totalWeight == 0) {
			std::cout << "warning: there is no valid possible expansion!" << std::endl;
			return std::pair<Mod, bool> (mod, false);
		}

		return std::pair<Mod, bool> (mod, true);
	}
	
	int getNumberOfToursPartition(uint pid, uint total) {
		if (TOURS_DIST_TYPE==-1) {
			return std::ceil((double)this->MAX_NUM_TOURS/(double)total);
		}
		else if (TOURS_DIST_TYPE==1) {
			return getNumberOfToursPartitionL(pid, total);
		}
		else if (TOURS_DIST_TYPE==2) {
			return getNumberOfToursPartitionQ(pid, total);
		}
		else {
			std::cout << "error: TOURS_DIST_TYPE unknown!" << std::endl;
			exit(1);
		}
	}

	uint getNumberOfToursPartitionL(uint pid, uint total) {
		double factor = (double)(this->MAX_NUM_TOURS*2)/((total+1)*total);
		return std::ceil((double)(total-pid)*factor);
	}

	uint getNumberOfToursPartitionQ(uint pid, uint total) {
		double factor = (double)(this->MAX_NUM_TOURS*6)/ ((double)(2*total+1)*total*(total+1));
		return std::ceil((double)std::pow(total-pid,2)*factor);
	}

};

#endif
