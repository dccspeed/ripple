#include "RippleEmbeddingSpace.h"
#include <unistd.h>
#include <chrono>
#include "tbb/compat/thread"
#include "utils.h"

template <class T, class A>
thread_local std::unordered_map<uint, int> EmbeddingSpaceMSK<T,A>::initialPartitionCounter = std::unordered_map<uint, int> (); 

template <class T, class A>
EmbeddingSpaceMSK<T,A>::EmbeddingSpaceMSK(int s, Graph *g): EmbeddingSpaceRGPM<T,A>(s,g) {
	this->nthreads = 1;
};

template <class T, class A>
EmbeddingSpaceMSK<T,A>::EmbeddingSpaceMSK(int s, Graph *g, std::string conf, std::string configString): EmbeddingSpaceMSK<T,A>(s,g)  {
	Config::load(conf,configString);
	Config::print();
	this->checkConfigFile();
	this->checkConfigFileRGPM();
	if (Config::existKey(std::string("MAX_RESERVOIR_SIZE"))) {
		MAX_RESERVOIR_SIZE = Config::getKeyAsInt(std::string("MAX_RESERVOIR_SIZE"));
	}
	if (Config::existKey(std::string("INIT_SUPERNODE_MAX_SCORE"))) {
		INIT_SUPERNODE_MAX_SCORE = Config::getKeyAsDouble(std::string("INIT_SUPERNODE_MAX_SCORE"));
	}
	if (Config::existKey(std::string("INIT_SUPERNODE_MIN_SCORE"))) {
		INIT_SUPERNODE_MIN_SCORE = Config::getKeyAsDouble(std::string("INIT_SUPERNODE_MIN_SCORE"));
	}
	if (Config::existKey(std::string("NUM_INITIAL_PARTITIONS"))) {
		NUM_INITIAL_PARTITIONS = Config::getKeyAsInt(std::string("NUM_INITIAL_PARTITIONS"));
	}
	if (Config::existKey(std::string("INITIAL_PARTITION_SIZE"))) {
		INITIAL_PARTITION_SIZE = Config::getKeyAsInt(std::string("INITIAL_PARTITION_SIZE"));
	}
	if (Config::existKey(std::string("FORCE_PARTITION_RETURN"))) {
		FORCE_PARTITION_RETURN = Config::getKeyAsBoolean(std::string("FORCE_PARTITION_RETURN"));
	}
        if (Config::existKey(std::string("PARTITION_RETURN_REJECTION"))) {
        	PARTITION_RETURN_REJECTION = Config::getKeyAsBoolean(std::string("PARTITION_RETURN_REJECTION"));
    	}
    	if (Config::existKey(std::string("RANDOMIZE_SCORES"))) {
    	    RANDOMIZE_SCORES = Config::getKeyAsBoolean(std::string("RANDOMIZE_SCORES"));
    	}
	if (Config::existKey(std::string("PRE_RESERVOIR_TYPE"))) {
		PRE_RESERVOIR_TYPE = Config::getKeyAsInt(std::string("PRE_RESERVOIR_TYPE"));
	}
	if (Config::existKey(std::string("TOURS_DIST_TYPE"))) {
		TOURS_DIST_TYPE = Config::getKeyAsInt(std::string("TOURS_DIST_TYPE"));
	}
	if (Config::existKey(std::string("PARTITION_REDUCTION_FACTOR"))) {
		PARTITION_REDUCTION_FACTOR = Config::getKeyAsInt(std::string("PARTITION_REDUCTION_FACTOR"));
	}
	if (Config::existKey(std::string("PARTITION_ERROR_BOUND"))) {
		PARTITION_ERROR_BOUND = Config::getKeyAsDouble(std::string("PARTITION_ERROR_BOUND"));
	}
};


template <class T, class A>
EmbeddingSpaceMSK<T,A>::EmbeddingSpaceMSK(int s, Graph *g, std::string conf, std::string configString, int t): EmbeddingSpaceMSK<T,A>(s,g,conf, configString)  {
	this->nthreads = t;
	std::cout << "running with " << this->nthreads << " threads!" << std::endl;
}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::run_exact_rec(T &e, int wordId) {
        e.addWord(wordId);
        if (e.getNumWords()==this->size) {
		int pid = getPartitionId(e);
		partitionNumEmbs[pid]++;
                if (!this->USE_PSRW_ESTIMATOR) {
                        double bias = 1;
                        earray<uint> *phash = this->getPatternHash(e);
                        //std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator itPattern = this->total_counts_estimate.find(phash);
                        std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(phash);
                        if (itPattern == this->total_counts_estimate.end()) {
                                this->total_counts_estimate.insert(std::make_pair(phash, (float128) bias));
                        } else {
                                itPattern->second += bias;
                        }
                }
                std::vector<Mod> mods = this->computeAllEmbeddingNeighborhood(e);
                for (int i = 0; i < (int) mods.size(); i++) {
                        e.replaceWord(mods[i].rmId, mods[i].addId);
			int _pid = getPartitionId(e);
                        if (this->USE_PSRW_ESTIMATOR) {
                                earray<uint> *phash = this->getPatternHash(e, mods[i].rmId);
				//double bias = 1./REDUCTION_FACTOR;
                                double bias = this->getPSRWBias(e, mods[i].rmId);
				//if (this->getPSRWBias(*phash, this->size+1) != bias) { std::cout << "bias different!"; exit(1); };
				bias*=0.5;

                                //std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator itPattern = this->total_counts_estimate.find(*phash);
                        	std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(phash);
                                if (itPattern == this->total_counts_estimate.end()) {
                                        //this->total_counts_estimate.insert(std::make_pair(*phash, (float128) bias));
                                        this->total_counts_estimate.insert(std::make_pair(phash, (float128) bias));
                                } else {
                                        itPattern->second += bias;
                                }
                        }
			if (_pid>pid) {
				size_t idx = getMatrixPositionInArray(pid, _pid);
				updatePartitionRelationReservoir(idx, e);
			}
                        e.replaceWord(mods[i].addId, mods[i].rmId);
                }
        }
        else {
                std::unordered_set<int> expansions = e.getValidElementsForExpansion();
                for (std::unordered_set<int>::iterator it = expansions.begin(); it!= expansions.end(); it++) {
                        if (e.isCanonicalEmbeddingWithWord(*it)) {
                                this->run_exact_rec(this->currEmbedding, *it);
                        }
                }
        }
        e.removeLastWord();
}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::run_exact() {
        if (this->USE_PSRW_ESTIMATOR){
                // Since random walks need to be done on a smaller HON
                this->size -= 1;
        }
	computeLayersBFS(this->currEmbedding, this->g);
	
	partitionNumEmbs.resize(partition2layerMap.size(), 0.);

        this->currEmbedding.reset();
        int numWords = this->currEmbedding.getTotalNumWords();
        for (int i = 0; i < numWords; i++) {
                run_exact_rec(this->currEmbedding, i);
        }

	for (uint i = 0; i < partition2layerMap.size(); i++) {
		float128 current_degree = 0;
		for (uint j = 0; j < i; j++) {
			size_t idx = getMatrixPositionInArray(j, i);
			current_degree += getDegreePartitionRelation(idx);
			std::cout << "degree between partitions " << j << " " << i << " " << getDegreePartitionRelation(idx) << std::endl;
		}
		std::cout << "number of subgraphs in partition " << i << ": " << partitionNumEmbs[i] << std::endl;
		std::cout << "degree arriving in partition " << i << ": " << current_degree << std::endl;
	}

	for (std::pair<earray<uint>*, float128> patternPair: this->total_counts_estimate) {
		if (this->USE_PSRW_ESTIMATOR) {
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size+1, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		else {
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		this->total_count_estimate+=patternPair.second;
	}
	
	std::cout << "total final count estimate: " << std::fixed << this->total_count_estimate << " or " << std::scientific << this->total_count_estimate << std::endl;

}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::fill_reservoir_exact_rec(T &e, int wordId) {
        e.addWord(wordId);
        if (e.getNumWords()==this->size) {
                int pid = getPartitionId(e);
                std::vector<Mod> mods = this->computeAllEmbeddingNeighborhood(e);
                for (int i = 0; i < (int) mods.size(); i++) {
                        e.replaceWord(mods[i].rmId, mods[i].addId);
                        int _pid = getPartitionId(e);
                        if (_pid>pid) {
                                size_t idx = getMatrixPositionInArray(pid, _pid);
                                updatePartitionRelationReservoir(idx, e);
                        }
                        e.replaceWord(mods[i].addId, mods[i].rmId);
                }
        }
        else {
                std::unordered_set<int> expansions = e.getValidElementsForExpansion();
                for (std::unordered_set<int>::iterator it = expansions.begin(); it!= expansions.end(); it++) {
                        if (e.isCanonicalEmbeddingWithWord(*it)) {
                                this->fill_reservoir_exact_rec(this->currEmbedding, *it);
                        }
                }
        }
        e.removeLastWord();
}


template <class T, class A>
void EmbeddingSpaceMSK<T,A>::fill_reservoir_exact() {
        this->currEmbedding.reset();
        
	int numWords = this->currEmbedding.getTotalNumWords();
        for (int i = 0; i < numWords; i++) {
                fill_reservoir_exact_rec(this->currEmbedding, i);
        }

	for (uint i = 0;  i < partition2layerMap.size(); i++){ 
		for (uint j = i;  j < partition2layerMap.size(); j++) {
                	size_t idx = getMatrixPositionInArray(i, j);
			std::cout << "reservoir partition relation " << i << " " << j << " size: " <<  getReservoirPartitionRelationSize(idx) << std::endl;
		}
	}
}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::fill_reservoir_rw() {
	//update reservoirs 
	for (uint i = 0; i < numInitialSuperNodePartitions; i++) {
		std::cout << "processing partition " << i << " with brute force for fill reservoir!" << std::endl;
		TourStatsWords firstStats = groupEstimateUsingBruteForceReservoir(i, partition2layerMap.size());
		
		for (uint j = numInitialSuperNodePartitions; j < partition2layerMap.size(); j++) {
			size_t idx = getMatrixPositionInArray(i, j);
			float128 relation_degree = getReservoirPartitionTotalSize(idx);
			std::cout << "partition relation idx: " << idx << " pid1 " << i << " pid2: " << j << " degree: " << relation_degree << std::endl;
			setDegreePartitionRelation(idx, relation_degree);
		}
	}

	//set first partition idx to be estimate
	uint current_partition_idx = numInitialSuperNodePartitions;

	int skip_partitions = 0;

	while (current_partition_idx < partition2layerMap.size()) {
		//processing partition
		//std::cout << "current rw partition: " << current_partition_idx << std::endl;
		//initialize current degree of the super node
		float128 current_degree = 0.;
		for (uint i = 0; i < current_partition_idx; i++) {
			size_t idx = getMatrixPositionInArray(i, current_partition_idx);
			current_degree += getDegreePartitionRelation(idx);
		}

		//if current degree is zero, this means that we did not visit any subgraph in the current_partition_idx
		if (current_degree == 0.) {
			skip_partitions++;
			//go to the next partition
			current_partition_idx++;
			continue;
		}

		uint numTours = getNumberOfToursPartition(current_partition_idx, partition2layerMap.size());
		
		//compute tours for estimate
		for (int i = 0; i < this->nthreads; i++) this->tourStatsThreads[i].reset();
		TourStatsWords tourStats = groupEstimateAllToursReservoir(current_partition_idx, current_degree, partition2layerMap.size(), numTours);

		//updating degrees and reservoirs from current_partition_idx to further partitions
		for (uint i = current_partition_idx + 1; i < partition2layerMap.size(); i++) {
			size_t idx = getMatrixPositionInArray(current_partition_idx, i);
			float128 relation_degree = current_degree / (float128) numTours * getReservoirPartitionTotalSize(idx);
			setDegreePartitionRelation(idx, relation_degree);  
			std::cout << "partition relation idx: " << idx << " pid1 " << current_partition_idx << " pid2: " << i << " degree: " << relation_degree << std::endl;
		}
			
		//go to the next partition
		current_partition_idx++;
	}

}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::run_rw() {
	if (this->USE_PSRW_ESTIMATOR){
		// Since random walks need to be done on a smaller HON
		this->size -= 1;
	}

	if (!this->init_rw()) {
		std::cout << "init fail!" << std::endl;
		return;
	}
	
	computeLayersBFS(this->currEmbedding, this->g);
	for (int i = 0; i < this->nthreads; i++) this->tourStatsThreads.push_back(TourStatsWords (partition2layerMap.size(), this->MAX_RESERVOIR_SIZE));

	size_t max_idx = getMatrixPositionInArray(partition2layerMap.size()-2,partition2layerMap.size()-1) +1;
	//partitionRelationMap.resize(max_idx, std::pair<float128, ReservoirWords> (0,ReservoirWords(this->MAX_RESERVOIR_SIZE)));


	for (uint i = 0; i < max_idx; i++) {
		partitionRelationMap.emplace_back(std::pair<float128, ReservoirWords> (0,ReservoirWords(i, this->MAX_RESERVOIR_SIZE)));
	}


	if (PRE_RESERVOIR_TYPE==1) {
		fill_reservoir_rw();
	}
	else if (PRE_RESERVOIR_TYPE==2) {
		fill_reservoir_exact();
	}
	else if (PRE_RESERVOIR_TYPE!=-1) {
		std::cout << "error: pre reservoir type unknown" << std::endl;
		exit(1);
	}


	//update reservoirs and estimates
	for (uint i = 0; i < numInitialSuperNodePartitions; i++) {
		std::cout << "processing partition " << i << " with brute force!" << std::endl;
		TourStatsWords firstStats = groupEstimateUsingBruteForce(i, partition2layerMap.size());
		std::cout << "number of subgraphs in partition " << i << " with brute force: " << firstStats.n << std::endl;

		//updating counts
		for (std::pair<earray<uint>*, float128> patternPair : firstStats.ns) {
			std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(patternPair.first);
			if (itPattern == this->total_counts_estimate.end()) {
				this->total_counts_estimate.insert(std::make_pair(patternPair.first, patternPair.second));
			} else {
				itPattern->second += patternPair.second;
			}
		}	    
		//updating degree and reservoir if its necessary
		if (PRE_RESERVOIR_TYPE == -1) {
			for (uint j = numInitialSuperNodePartitions; j < partition2layerMap.size(); j++) {
				size_t idx = getMatrixPositionInArray(i, j);
				float128 relation_degree = getReservoirPartitionTotalSize(idx);
				std::cout << "partition relation idx: " << idx << " pid1 " << i << " pid2: " << j << " degree: " << relation_degree << std::endl;
				setDegreePartitionRelation(idx, relation_degree);  
			}
		}
	}
	//set first partition idx to be estimate
	uint current_partition_idx = numInitialSuperNodePartitions;
	int skip_partitions = 0;

	while (current_partition_idx < partition2layerMap.size()) {
		//processing partition
		std::cout << "current rw partition: " << current_partition_idx << std::endl;

		//initialize current degree of the super node
		float128 current_degree = 0.;
		for (uint i = 0; i < current_partition_idx; i++) {
			size_t idx = getMatrixPositionInArray(i, current_partition_idx);
			current_degree += getDegreePartitionRelation(idx);
			std::cout << "degree between partitions " << i << " " << current_partition_idx << " " << getDegreePartitionRelation(idx) << std::endl;
		}
		//if current degree is zero, this means that we did not visit any subgraph in the current_partition_idx
		if (current_degree == 0.) {
			skip_partitions++;
			//go to the next partition
			current_partition_idx++;
			continue;
		}

		
		for (int i = 0; i < this->nthreads; i++) this->tourStatsThreads[i].reset();
		TourStatsWords tourStats(partition2layerMap.size(), this->MAX_RESERVOIR_SIZE);
		//get initial number of tours
		uint numTours = getNumberOfToursPartition(current_partition_idx, partition2layerMap.size());
		float128 partition_error = 0;

		//compute tours for estimate
		do {
			std::cout << "number of tours of partition " << current_partition_idx << ": " << numTours << std::endl;
			tourStats += groupEstimateAllTours(current_partition_idx, current_degree, partition2layerMap.size(), numTours); 
			for (int i = 0; i < this->nthreads; i++) this->tourStatsThreads[i].reset();
			partition_error = sqrt(tourStats.n2/tourStats.numTours - pow(tourStats.n/tourStats.numTours, 2.)) / (tourStats.n/tourStats.numTours) / std::sqrt(tourStats.numTours);
			std::cout << "partition error " << current_partition_idx << " " <<  partition_error << std::endl;
			
			numTours = this->MAX_NUM_TOURS_BATCH;
		} while (partition_error>PARTITION_ERROR_BOUND && PARTITION_ERROR_BOUND>0);
		
		std::cout << "FINALTOUR PARTITION " << current_partition_idx << " " << tourStats << " REJECTION COUNTERS: " << this->REJECTION_COUNTER1 << "/" << this->REJECTION_COUNTER2 << "/" << REJECTION_COUNTER3 << "/" << REJECTION_COUNTER4 << std::endl;

		//update counts
		std::cout << "degree arriving in partition " << current_partition_idx << ": " << current_degree << std::endl;
		std::cout << "number of subgraphs in partition " << current_partition_idx << ": " << current_degree / (float128) tourStats.numTours * tourStats.n << std::endl;

		for (std::pair<earray<uint>*, float128> patternPair : tourStats.ns) {
			std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(patternPair.first);
			float128 patternCount = current_degree / (float128) tourStats.numTours * patternPair.second;
			if (itPattern == this->total_counts_estimate.end()) {
				this->total_counts_estimate.insert(std::make_pair(patternPair.first, patternCount));
			} else {
				itPattern->second += patternCount;
			}
		}

               
		if (PRE_RESERVOIR_TYPE == -1) {
                       for (uint i = current_partition_idx + 1; i < partition2layerMap.size(); i++) {
                                size_t idx = getMatrixPositionInArray(current_partition_idx, i);
				float128 relation_degree = current_degree / (float128) tourStats.numTours * getReservoirPartitionTotalSize(idx);
                                setDegreePartitionRelation(idx, relation_degree);  
				std::cout << "partition relation idx: " << idx << " pid1 " << current_partition_idx << " pid2: " << i << " degree: " << relation_degree << " reservoir total: " << getReservoirPartitionTotalSize(idx) << std::endl;
                       }
               }


		//clean previous reservoirs
		for (uint i = 0; i < current_partition_idx; i++) {
			size_t idx = getMatrixPositionInArray(i, current_partition_idx);
			resetPartitionRelation(idx);
		}

		//go to the next partition
		current_partition_idx++;
	}

	if (skip_partitions > 0)
		std::cout << " warning: " << skip_partitions << " partitions were not sampled!" << std::endl;
	//for (std::pair<earray<uint>, float128> patternPair: this->total_counts_estimate) {
	for (std::pair<earray<uint>*, float128> patternPair: this->total_counts_estimate) {
		if (this->USE_PSRW_ESTIMATOR) {
			//patternPair.second=REDUCTION_FACTOR*(patternPair.second*this->getPSRWBias(*patternPair.first, this->size+1));
			//std::cout << "phash: " << Canonical::getMotivoHash(patternPair.first, this->size+1, 30)  << " bias: " << this->getPSRWBias(*patternPair.first, this->size+1) << std::endl;
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size+1, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		else {
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		this->total_count_estimate+=patternPair.second;
	}
	
	std::cout << "total final count estimate: " << std::fixed << this->total_count_estimate << " or " << std::scientific << this->total_count_estimate << std::endl;
}

template <class T, class A>
T EmbeddingSpaceMSK<T,A>::getRandomEmbeddingInLevel(uint partition_idx) {
	uint pid = 0;
	bool b = false;
	float128 acc=0.;
	for (uint i=0; i < partition_idx; i++) {
		size_t idx = getMatrixPositionInArray(i,partition_idx);
		if (getDegreePartitionRelation(idx)==0) continue;

		double p = Randness::getInstance()->random_uni01();
		acc+=getDegreePartitionRelation(idx);
		if (p<=getDegreePartitionRelation(idx)/acc) {
			pid = i;
			b = true;
		}
	}

	if (!b) {
		std::cout << "error: can not get a random embedding from reservoir!" << std::endl;
		exit(1);
	}

	size_t idx = getMatrixPositionInArray(pid,partition_idx);
	if (pid<numInitialSuperNodePartitions)
		return T(this->g, getReservoirPartitionRelationRandomElement(idx, true), this->size);
	else
		return T(this->g, getReservoirPartitionRelationRandomElement(idx, false), this->size);

}

template <class T, class A>
TourStatsWords EmbeddingSpaceMSK<T, A>::groupEstimateAllTours(uint partition_idx, float128 partition_degree, uint numPartitions, int nTours) {
	TourStatsWords finalTourStats(numPartitions, this->MAX_RESERVOIR_SIZE);
	tbb::task_scheduler_init init(this->nthreads);

	auto start = std::chrono::high_resolution_clock::now(); 


	tbb::parallel_for(
			tbb::blocked_range<int>(0, nTours),
			[&](const tbb::blocked_range<int> &r)  {

			int id = tbb::this_task_arena::current_thread_index();
			//std::cout << "using thread id " << id << " number of tourStats " << this->tourStatsThreads.size() <<  " range " << r.end()-r.begin() << std::endl;
			for (int i = r.begin(); i < r.end(); i++)
			this->groupEstimateUsingTour(partition_idx, partition_degree, numPartitions, i, this->tourStatsThreads[id]);

			});

	auto stop = std::chrono::high_resolution_clock::now(); 
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
	std::cout << "Time taken by groupEstimateAllTours: " << duration.count() << " microseconds" << std::endl; 

	for (TourStatsWords tourStats : this->tourStatsThreads) finalTourStats+=tourStats;
	//std::cout << "PartitionId: " << partition_idx << " " <<  finalTourStats << " REJECTION COUNTERS: " << this->REJECTION_COUNTER1 << "/" << this->REJECTION_COUNTER2 << "/" << REJECTION_COUNTER3 << "/" << REJECTION_COUNTER4 << std::endl;
	return finalTourStats;
	//return this->tourStatsThreads[finalThreadId];
}

template <class T, class A>
TourStatsWords &EmbeddingSpaceMSK<T,A>::groupEstimateUsingTour(uint partition_idx, float128 partition_degree, uint numPartitions, int rwId, TourStatsWords &tourStats) {

	//copy embeddings to modify
	T embeddingCopy = getRandomEmbeddingInLevel(partition_idx);

	int steps = 1;
	int stepsRW = 1;

	std::pair<Mod, bool> mod;
	ModSet mods;

	float128 n = 0;

	//std::cout << "starting rw from embedding: " << embeddingCopy << std::endl;
	bool reachedSupernode = false;
	bool exitPartition = false; 
	do {
		//get next modification in advance
		if (!exitPartition) {
			mod = this->getNextRandomModification(embeddingCopy, mods);
			if (!this->USE_PSRW_ESTIMATOR) {
				//std::cout << "embedding : " << embeddingCopy << " weight: " << mod.first.totalWeight << std::endl;
				n+=1./mod.first.totalWeight;
				tourStats.insert(this->getPatternHash(embeddingCopy), 1./mod.first.totalWeight);
			}
			stepsRW++;
		}
		else {
			if (PRE_RESERVOIR_TYPE==-1) {
				//first fill the reservoir
				//std::cout << "embedding is in further space! returning ..." << std::endl;
				uint pid = getPartitionId(embeddingCopy);
				size_t idx = getMatrixPositionInArray(partition_idx, pid);
				updatePartitionRelationReservoir(idx,embeddingCopy);
			}

			//force to return to the previous partition
			if (FORCE_PARTITION_RETURN==true) {
				mod.first = mod.first.inverse();
				mod.first.totalWeight = 1;
			}
			else {
				if (PARTITION_RETURN_REJECTION==true) mod = getNextRandomModificationByRejectionToSN(embeddingCopy, partition_idx); 
				else mod = getNextRandomModificationToSN(embeddingCopy, partition_idx); 
			}
		}


		//walk to the next embedding
		embeddingCopy.replaceWord(mod.first.rmId, mod.first.addId);

		//check if the tour is in super node
		reachedSupernode = isInPreviousLevels(partition_idx, embeddingCopy);

		//check if the tour is in the rw space or in the further
		exitPartition = !isInPreviousLevels(partition_idx + 1, embeddingCopy);

		if (!reachedSupernode && this->USE_PSRW_ESTIMATOR) {
			// Edges incident on the supernode have been accounted for already
			earray<uint> *phash = this->getPatternHash(embeddingCopy, mod.first.rmId);
			//double bias = 1./REDUCTION_FACTOR;
			double bias = this->getPSRWBias(embeddingCopy, mod.first.rmId);
			//if (this->getPSRWBias(*phash, this->size+1) != bias) { std::cout << "bias different!"; exit(1); };

			bias*=0.5;
			n += bias;
			tourStats.insert(phash, bias);
		}

		steps++;

	} while (steps < this->MAX_TOUR_STEPS && !reachedSupernode);

	tourStats.n += n;
	tourStats.n2 += pow(n,2);	
	tourStats.steps += steps;
	tourStats.stepsRW += stepsRW;
	tourStats.numTours += 1;
	tourStats.stepsSecMoment+=std::pow((double)steps, 2)/REDUCTION_FACTOR;
	tourStats.ret = (steps < this->MAX_TOUR_STEPS) && tourStats.ret;
	//std::cout << "PartitionId: " << partition_idx << " TourNumber: " << rwId << " " <<  tourStats << std::endl;
	return tourStats;	
}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::groupEstimateUsingBruteForceUtil(T &e, int wordId, uint partition_idx, std::unordered_set<int> &l, TourStatsWords &stats) {
	e.addWord(wordId);

	if (e.getNumWords()==this->size) {
		//std::cout << "--embedding found " << e << std::endl;
		if (!this->USE_PSRW_ESTIMATOR){
			stats.n+=1.;
			stats.n2+=1;
			stats.insert(this->getPatternHash(e), 1.);
		}
		std::vector<Mod> mods = this->computeAllEmbeddingNeighborhood(e);
		for (int i = 0; i < (int) mods.size(); i++) {
			stats.steps+=1;

			//processing the neighbor
			e.replaceWord(mods[i].rmId, mods[i].addId);

			//get which partition this embedding belongs
			uint pid = getPartitionId(e);

			//filling the hashmap for counting in PSRW
			if (this->USE_PSRW_ESTIMATOR){
				earray<uint> *phash = this->getPatternHash(e, mods[i].rmId);
				double bias = this->getPSRWBias(e, mods[i].rmId);
				
				// To account for double counting of edges within partition.  Assuming that edges between brute forch partitions don't exist
				if (pid == partition_idx) bias *= 0.5;
				
				stats.n += bias;
				stats.n2 += std::pow(bias,2);
				stats.insert(phash, bias);
			}

			//fill the reservoirs of neighbors to perform RWTs
			if (PRE_RESERVOIR_TYPE==-1 && pid > partition_idx) {
				size_t idx = getMatrixPositionInArray(partition_idx, pid);
				updatePartitionRelationReservoir(idx, e);
			}
			e.replaceWord(mods[i].addId, mods[i].rmId);

		}
	}
	else {
		std::unordered_set<int> expansions = e.getValidElementsForExpansionWith(l);
		for (std::unordered_set<int>::iterator it = expansions.begin(); it!= expansions.end(); it++) {
			if (e.isCanonicalEmbeddingWithWord(*it)) {
				groupEstimateUsingBruteForceUtil(e, *it, partition_idx, l, stats);
			}
		}
	}
	e.removeLastWord();
}

template <class T, class A>
TourStatsWords EmbeddingSpaceMSK<T,A>::groupEstimateUsingBruteForce(uint partition_idx, uint numPartitions) {
	TourStatsWords stats(numPartitions, this->MAX_RESERVOIR_SIZE);

	std::unordered_set<int> &l = layers[partition_idx];
	if ((int) l.size() < this->size) return stats;
	std::cout << "=brute force computing partition " << partition_idx  << std::endl;
	T e(this->g);
	for (int i : l) {
		std::cout << "==processing node " << i << " degree "<< this->g->getDegreeOfNodeAt(i) << std::endl;
		groupEstimateUsingBruteForceUtil(e, i, partition_idx, l, stats);
	}

	return stats;
}

template <class T, class A>
bool EmbeddingSpaceMSK<T,A>::isInPreviousLevels(uint idx, T &e) {
	return (getPartitionId(e)<idx);
}

template <class T, class A>
std::vector<std::unordered_set<int>> EmbeddingSpaceMSK<T,A>::buildInitialPartitions(boost::dynamic_bitset<> &lroot, boost::dynamic_bitset<> &lreach, Graph *g, int numPartitions, int maxsize, int minscore, int maxscore, std::vector<int> &scores) {
	std::vector<std::unordered_set<int>> newPartitions;
	//scores = g->getNodesCoreness();
	//scores = g->getNodesLayers();
	//scores = g->getNodesDegrees();
	//scores = g->getNodesColors();
	//scores = g->getHopDistNodes(pneighs);

	std::vector<std::pair<int,int>> score_rank;
	for (int i = 0; i < g->getNumberOfNodes(); i++) {
		score_rank.push_back(std::pair<int,int>(i,scores[i]));
	}
	std::stable_sort(score_rank.begin(), score_rank.end(), [](auto &left, auto &right) {
			return left.second > right.second;
			});

	for (int rank_pos = 0; rank_pos<g->getNumberOfNodes(); rank_pos++) {
		if ((int) newPartitions.size() >= numPartitions) break;
		//std::priority_queue<std::pair<int,double>, std::vector< std::pair<int,double>>, CompareIntDoublePairInc> neighQueue;
		std::priority_queue<std::pair<int,double>, std::vector< std::pair<int,double>>, CompareIntDoublePairDec> neighQueue;
		boost::dynamic_bitset<> lqueue(g->getNumberOfNodes());
		int nodeRoot = 	score_rank[rank_pos].first;
		int nodeScore = score_rank[rank_pos].second;

		if (nodeScore<minscore||nodeScore>maxscore) continue;
		if (lroot.test(nodeRoot) || lreach.test(nodeRoot)) continue;

		std::unordered_set<int> partition;
		neighQueue.push(std::pair<int, double> (nodeRoot, scores[nodeRoot]));

		//build initial partition from nodeRoot
		while (!neighQueue.empty() && (int) partition.size()<maxsize) {
			std::pair<int, double> top = neighQueue.top();
			neighQueue.pop();
			std::cout << "#inserting " << top.first << " partition size: " << partition.size() << std::endl;
			partition.insert(top.first);
			std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(top.first);
			for (int n : neigh) {
				if (!lroot.test(n) && !lreach.test(n) && !lqueue.test(n)) {
					if (scores[n]>=minscore && scores[n]<=maxscore) {
						neighQueue.push(std::pair<int, double> (n, scores[n]));
						lqueue.set(n, true);
					}
				}
			}
		}
		if ((int) partition.size() >= this->size) {
			uint totalDegree=0;
			std::vector<uint> degrees;
			for (int i : partition) {
				std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(i);
				totalDegree+=neigh.size();
				degrees.push_back(neigh.size());
				lroot.set(i, true);
				for (int n : neigh) lreach.set(n, true);
			}
			std::cout << "creating partition with root: " << nodeRoot << " partition size: " << partition.size() << " total degree: " << totalDegree << " degrees:";
			for (uint i : degrees) std::cout << " " <<  i;
			std::cout << std::endl;
			newPartitions.push_back(partition);
		}
		//else {
		//	std::cout << "discarding partition with root: " << nodeRoot << " partition size: " << partition.size() << std::endl;
		//}
		//last_initial_root_pos = rank_pos;
	}

	return newPartitions;
}

template <class T, class A>
std::vector<std::unordered_set<int>> EmbeddingSpaceMSK<T,A>::buildAllInitialPartitionsBFS(boost::dynamic_bitset<> &lroot, boost::dynamic_bitset<> &lreach, Graph *g, int numPartitions, int maxsize, int minscore, int maxscore, std::vector<int> &scores) {
	std::cout << "Bulding ALL partition seeds with BFS!" << std::endl;
	boost::dynamic_bitset<> lcheck(g->getNumberOfNodes());
	std::vector<std::unordered_set<int>> newPartitions;

	std::priority_queue<std::pair<int,double>, std::vector< std::pair<int,double>>, CompareIntDoublePairInc> neighGlobalQueue;
	//std::vector<std::pair<int,int>> score_rank;
	for (int i = 0; i < g->getNumberOfNodes(); i++) {
		//score_rank.push_back(std::pair<int,int>(i, scores[i]));
		//std::cout << "node: " << i << " score: " << scores[i] << std::endl;
		neighGlobalQueue.push(std::pair<int, double> (i, (double) scores[i]));
	}

	while (!neighGlobalQueue.empty() && (int) newPartitions.size()<numPartitions) {
		std::pair<int, double> topGlobal = neighGlobalQueue.top();
		int nodeRoot = topGlobal.first;
		int nodeScore = topGlobal.second;
		neighGlobalQueue.pop();

		//std::cout << "TRY bulding partition seed! nodeRoot is valid: " << nodeRoot << std::endl;

		if (lcheck.test(nodeRoot)) continue;
		lcheck.set(nodeRoot, true);

		std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(nodeRoot);
		//insert neighbors of the root for futher checking
		for (int n : neigh) {
			if (!lroot.test(n) && !lreach.test(n)) {
				if (scores[n]>=minscore && scores[n]<=maxscore)
					neighGlobalQueue.push(std::pair<int, double> (n, scores[n]));
			}
		}

		if (nodeScore<minscore || nodeScore>maxscore) continue;
		if (lroot.test(nodeRoot) || lreach.test(nodeRoot)) continue;

		std::unordered_set<int> partition;
		//std::priority_queue<std::pair<int,double>, std::vector< std::pair<int,double>>, CompareIntDoublePairInc> neighQueue;
		std::deque<std::pair<int,double>> neighQueue;
		neighQueue.push_front(std::pair<int, double> (nodeRoot, scores[nodeRoot]));

		//build initial partition from nodeRoot
		while (!neighQueue.empty() && (int) partition.size()<maxsize) {
			std::pair<int, double> top = neighQueue.back();
			//std::pair<int, double> top = neighQueue.top();
			//neighQueue.pop();
			neighQueue.pop_back();
			neigh = g->getNeighborhoodIdxVertexOfVertexAt(top.first);
			partition.insert(top.first);
			std::cout << "inserting " << top.first << "partition size: " << partition.size() << std::endl;
			for (uint n : neigh) {
				//std::cout << "=checking neighbor " << n << std::endl;
				if (!lroot.test(n) && !lreach.test(n) && partition.find(n)==partition.end()) {
					//std::cout << "=checking neighbor score: " << scores[n] << " " << maxscore << std::endl;
					if (scores[n]>=minscore && scores[n]<=maxscore)
						//neighQueue.push(std::pair<int, double> (n, scores[n]));
						neighQueue.push_front(std::pair<int, double> (n, scores[n]));
				}
			}
		}
		if ((int) partition.size() >= this->size) {
			std::cout << "creating partition with root: " << nodeRoot << " partition size: " << partition.size() << std::endl;
			for (int i : partition) {
				std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(i);
				lroot.set(i, true);
				for (int n : neigh) lreach.set(n, true);
			}
			newPartitions.push_back(partition);
		}
	}

	std::cout << "number of partitions seed built: " << newPartitions.size() << std::endl;

	return newPartitions;
}


template <class T, class A>
void EmbeddingSpaceMSK<T,A>::getInitialLayer(Graph *g, int maxNumRegions, int maxRegionSize, std::vector<int> &scores) {
	initialPartitionMap.resize(g->getNumberOfNodes(), 0);
	boost::dynamic_bitset<> lreach(g->getNumberOfNodes());
	boost::dynamic_bitset<> lroot(g->getNumberOfNodes());

	//create a rank based on the score of nodes
	std::vector<std::pair<int,int>> score_rank;
	for (int i = 0; i < g->getNumberOfNodes(); i++) {
		score_rank.push_back(std::pair<int,int>(i,scores[i]));
	}
	std::stable_sort(score_rank.begin(), score_rank.end(), [](auto &left, auto &right) {
			return left.second < right.second;
			});

	int rank_pos = 0;
	int numRegions = 0;
	while (numRegions<maxNumRegions && rank_pos<g->getNumberOfNodes()) {
		//create priority queue to create the better region possible
		std::priority_queue<std::pair<int,double>, std::vector< std::pair<int,double>>, CompareIntDoublePairInc> neighQueue;
		boost::dynamic_bitset<> lqueue(g->getNumberOfNodes());
		int nodeRoot =  score_rank[rank_pos].first;
		int nodeScore = score_rank[rank_pos].second;
		rank_pos++;

		if (lroot.test(nodeRoot) || lreach.test(nodeRoot)) continue;
		if (nodeScore<INIT_SUPERNODE_MIN_SCORE||nodeScore>INIT_SUPERNODE_MAX_SCORE) continue;
		//std::cout << "building region with root: " << nodeRoot << " node score: " <<  nodeScore << std::endl;

		//build initial layer from nodeRoot
		neighQueue.push(std::pair<int, double> (nodeRoot, scores[nodeRoot]));
		std::unordered_set<int> region;
		while (!neighQueue.empty() && (int) region.size()<maxRegionSize) {
			std::pair<int, double> top = neighQueue.top();
			neighQueue.pop();
			//std::cout << "#inserting " << top.first << " score: " << scores[top.first] << std::endl;
			region.insert(top.first);
			std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(top.first);
			for (int n : neigh) {
				if (!lroot.test(n) && !lreach.test(n) && !lqueue.test(n)) {
					if ((double)scores[n]>=INIT_SUPERNODE_MIN_SCORE && (double)scores[n]<=INIT_SUPERNODE_MAX_SCORE) {
						neighQueue.push(std::pair<int, double> (n, scores[n]));
						lqueue.set(n, true);
					}
				}
			}
		}
		//add region to layer
		if ((int) region.size() >= this->size) {
			for (int i : region) {
				initialPartitionMap[i] = numRegions;
				lroot.set(i, true);
				std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(i);
				for (int n : neigh) lreach.set(n, true);
			}
			initLayer.insert(region.begin(), region.end());
			std::cout << "adding region with root: " << nodeRoot << " region size: " << region.size() << std::endl;
			numRegions++;
		}
		else {
			std::cout << "discarding region with root: " << nodeRoot << " region size: " << region.size() << std::endl;
		}
	}


}



template <class T, class A>
void  EmbeddingSpaceMSK<T,A>::computeLayersBFS(T &e, Graph *g) {
    layerMap.resize(g->getNumberOfNodes(), 0);
        
    std::vector<int> scores;
    
    if (this->RANDOMIZE_SCORES){
	int maxDegree = g->getLargestNodeDegree();
	for (int i = 0; i < g->getNumberOfNodes();i++) 	scores.push_back(Randness::getInstance()->get_a_random_number(0, maxDegree));
    }
    else {
	    scores.insert(scores.begin(), g->getNodesDegrees().begin(), g->getNodesDegrees().end());
    }


	//get initial layer
	getInitialLayer(g, NUM_INITIAL_PARTITIONS, std::max(INITIAL_PARTITION_SIZE, this->size), scores);

	if (initLayer.empty()) {
		std::cout << "error: initial layer is empty! no seeds were found!" << std::endl;
		exit(1);
	} 

	std::cout << "initlayer: { ";
	for (int i : initLayer) std::cout << i << " ";
	std::cout << "}" << std::endl;

	//define the layer of each node
	std::unordered_set<int> acclayer;
	std::unordered_set<int> layerNeighbors;
	int layerId = 0;
	for (int i : initLayer) {
		layerMap[i] = layerId;
		acclayer.insert(i);
		layerNeighbors.erase(i);
		std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(i);
		for (int j : neigh) { 
			if (acclayer.find(j)==acclayer.end() && layerNeighbors.find(j)==layerNeighbors.end()) {
				layerNeighbors.insert(j);
			}
		}
	}

	while ((int) acclayer.size() != g->getNumberOfNodes()) {
		std::unordered_set<int> newNeighbors;

		//build next layer
		layerId++;
		//std::cout << "building next layer: " << layerId << std::endl;

		for (std::unordered_set<int>::iterator it = layerNeighbors.begin(); it != layerNeighbors.end(); it++) {
			//select one node to be part of this layer
			int i = *it;

			//add node in layer and remove from neighbors
			layerMap[i] = layerId;
			acclayer.insert(i);

			//check for new neighbors
			std::vector<int> &neigh = g->getNeighborhoodIdxVertexOfVertexAt(i);
			for (int j : neigh) { 
				if (acclayer.find(j)==acclayer.end() && layerNeighbors.find(j)==layerNeighbors.end()) {
					newNeighbors.insert(j);
				}
			}
		} 
		layerNeighbors.clear();
		layerNeighbors.insert(newNeighbors.begin(), newNeighbors.end());
	}

	layers.resize(layerId+1, std::unordered_set<int>());
	for (int i = 0; i < g->getNumberOfNodes(); i++) {
		//std::cout << "layerMap[" << i << "]: " << layerMap[i] << std::endl;
		layers[layerMap[i]].insert(i);
	}

	acclayer.clear();
	uint numPartitions=0;
	for (uint i = 0; i < layers.size(); i++) {
		std::unordered_set<int> &l = layers[i];
		acclayer.insert(l.begin(), l.end());
		if (i>0) {
			int numSupPart = this->size;
			layerNumPartitions.push_back(numSupPart);
		}
		else {
			layerNumPartitions.push_back(1);
		}
		for (uint j = 0; j < layerNumPartitions[i]; j++) {
			std::pair<int,int> partitionPar(i, j);
			layer2partitionMap[partitionPar] = numPartitions;
			partition2layerMap[numPartitions] = partitionPar;
			numPartitions++;
		}
		std::cout << "layer: " << i << " num nodes: " << layers[i].size() << " numSubPartitions: " << layerNumPartitions[i] << std::endl;
	}
	numInitialSuperNodePartitions=1;

	//create idx
	this->g->sortNeighborhoodIdx(layerMap, initialPartitionMap);
	for (int i = 0; i < g->getNumberOfNodes(); i++) {
		std::vector<std::pair<uint, uint>> aux(layers.size(), std::pair<uint, uint> (0,0));
		std::vector<int> &neigh = this->g->getNeighborhoodIdxVertexOfVertexAt(i);

		uint idx = 0;
		uint offset = 1;
		uint currLayerId = layerMap[neigh[0]];
		for (uint j = 1; j < neigh.size(); j++) {
			if (currLayerId != layerMap[neigh[j]]) {
				aux[currLayerId].first = idx;
				aux[currLayerId].second = offset;
				//std::cout << "currLayerId: " << currLayerId << " idx: " << aux[currLayerId].first << " offset: " << aux[currLayerId].second << std::endl; 
				idx=j;
				offset=0;
				currLayerId=layerMap[neigh[j]];
			}
			offset++;
		}
		//update the last sequence of layer id	
		aux[currLayerId].first = idx;
		aux[currLayerId].second = offset;
		nodeLayerIdx.push_back(aux);	
	}

}

template <class T, class A>
TourStatsWords EmbeddingSpaceMSK<T, A>::groupEstimateAllToursReservoir(uint partition_idx, float128 partition_degree, uint numPartitions, int nTours) {
	//TourStatsWords finalTourStats(numPartitions, this->MAX_RESERVOIR_SIZE);
	tbb::task_scheduler_init init(this->nthreads);

	
	tbb::parallel_for(
			tbb::blocked_range<int>(0, nTours),
			[&](const tbb::blocked_range<int> &r)  {

			int id = tbb::this_task_arena::current_thread_index();
			//std::cout << "using thread id " << id << " number of tourStats " << this->tourStatsThreads.size() <<  " range " << r.end()-r.begin() << std::endl;
			for (int i = r.begin(); i < r.end(); i++)
			this->groupEstimateUsingTourReservoir(partition_idx, partition_degree, numPartitions, i, this->tourStatsThreads[id]);

			});

	//for (TourStatsWords tourStats : this->tourStatsThreads) finalTourStats+=tourStats;
	// Reduce the reservoirs to some position in the vector (inplace reuce)	
	   int finalThreadId = tbb::parallel_reduce(
              tbb::blocked_range<int>(0,this->nthreads),
              -1, // invalid thread id
              [&](tbb::blocked_range<int> r, int threadId)
              {
                 int myThreadId = tbb::this_task_arena::current_thread_index();
                 return myThreadId;
              },
              [&](int threadId1, int threadId2) {
                 // always aggregate on first
                 this->tourStatsThreads[threadId1] += this->tourStatsThreads[threadId2];
		 this->tourStatsThreads[threadId2].reset();
                 return threadId1;
              }
        );
	
		
	//std::cout << "FINISHTOUR Reservoir PartitionId: " << partition_idx << "/" << partition2layerMap.size() << " Layer: " << partition2layerMap.find(partition_idx)->second.first << "/" <<  partition2layerMap.find(partition_idx)->second.second << " " <<  finalTourStats << " REJECTION COUNTERS: " << this->REJECTION_COUNTER1 << "/" << this->REJECTION_COUNTER2 << "/" << REJECTION_COUNTER3 << "/" << REJECTION_COUNTER4 << std::endl;
	//return finalTourStats;
        return this->tourStatsThreads[finalThreadId];

}

template <class T, class A>
TourStatsWords &EmbeddingSpaceMSK<T,A>::groupEstimateUsingTourReservoir(uint partition_idx, float128 partition_degree, uint numPartitions, int rwId, TourStatsWords &tourStats) {
	//copy embeddings to modify
	T embeddingCopy = getRandomEmbeddingInLevel(partition_idx);

	int steps = 1;
	int stepsRW = 1;

	std::pair<Mod, bool> mod;
	ModSet mods;

	//std::cout << "starting rw from embedding: " << embeddingCopy << std::endl;
	bool reachedSupernode = false;
	bool exitPartition = false; 
	do {
		//get next modification in advance
		if (!exitPartition) {
			mod = this->getNextRandomModification(embeddingCopy, mods);
		}
		else {
			stepsRW++;
			//first fill the reservoir
			uint pid = getPartitionId(embeddingCopy);
			size_t idx = getMatrixPositionInArray(partition_idx, pid);
			updatePartitionRelationReservoir(idx, embeddingCopy);

			//force to return to the previous partition
			mod.first = mod.first.inverse();
			mod.first.totalWeight = 1;
		}


		//walk to the next embedding
		embeddingCopy.replaceWord(mod.first.rmId, mod.first.addId);

		//check if the tour is in super node
		reachedSupernode = isInPreviousLevels(partition_idx, embeddingCopy);

		//check if the tour is in the rw space or in the further
		exitPartition = !isInPreviousLevels(partition_idx + 1, embeddingCopy);
		steps++;

	} while (steps < this->MAX_TOUR_STEPS && !reachedSupernode);

	tourStats.steps += steps;
	tourStats.stepsRW += stepsRW;
	tourStats.stepsSecMoment += std::pow((double)steps, 2)/REDUCTION_FACTOR;
	tourStats.ret = (steps < this->MAX_TOUR_STEPS) && tourStats.ret;

	return tourStats;	
}

template <class T, class A>
void EmbeddingSpaceMSK<T,A>::groupEstimateUsingBruteForceUtilReservoir(T &e, int wordId, uint partition_idx, std::unordered_set<int> &l, TourStatsWords &stats) {
	e.addWord(wordId);

	if (e.getNumWords()==this->size) {
		std::vector<Mod> mods = this->computeAllEmbeddingNeighborhood(e);
		for (int i = 0; i < (int) mods.size(); i++) {
			stats.steps+=1;
			//processing the neighbor
			e.replaceWord(mods[i].rmId, mods[i].addId);

			//get which partition this embedding belongs
			uint pid = getPartitionId(e);

			//fill the reservoirs of neighbors to perform RWTs
			if (pid > partition_idx) {
				size_t idx = getMatrixPositionInArray(partition_idx, pid);
				updatePartitionRelationReservoir(idx, e);
			}

			e.replaceWord(mods[i].addId, mods[i].rmId);
		}
	}
	else {
		std::unordered_set<int> expansions = e.getValidElementsForExpansionWith(l);
		for (std::unordered_set<int>::iterator it = expansions.begin(); it!= expansions.end(); it++) {
			if (e.isCanonicalEmbeddingWithWord(*it)) {
				groupEstimateUsingBruteForceUtilReservoir(e, *it, partition_idx, l, stats);
			}
		}
	}
	e.removeLastWord();
}

template <class T, class A>
TourStatsWords EmbeddingSpaceMSK<T,A>::groupEstimateUsingBruteForceReservoir(uint partition_idx, uint numPartitions) {
	TourStatsWords stats(numPartitions, this->MAX_RESERVOIR_SIZE);

	std::unordered_set<int> &l = layers[partition_idx];
	if ((int) l.size() < this->size) return stats;
	T e(this->g);
	for (int i : l) {
		groupEstimateUsingBruteForceUtilReservoir(e, i, partition_idx, l, stats);
	}

	return stats;
}

template class EmbeddingSpaceMSK<VertexInducedEmbedding, AggregatorPatternCounter>;
//template class EmbeddingSpaceMSK<EdgeInducedEmbedding, AggregatorPatternCounter>;

