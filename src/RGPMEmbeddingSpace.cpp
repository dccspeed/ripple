#include "RGPMEmbeddingSpace.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_cdf.h>
#include <unordered_map>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>
#include <limits>
#include <utils.h>
#include <omp.h>

template <class T, class A>
EmbeddingSpaceRGPM<T,A>::EmbeddingSpaceRGPM(int s, Graph *g): EmbeddingSpace<T,A>(s,g) {
	snSize = 1;
        nthreads=1;	
	setStackSize();
};

template <class T, class A>
EmbeddingSpaceRGPM<T,A>::EmbeddingSpaceRGPM(int s, int sns, Graph *g): EmbeddingSpaceRGPM<T,A>(s,g) {
	snSize = sns;
}

template <class T, class A>
EmbeddingSpaceRGPM<T,A>::EmbeddingSpaceRGPM(int s, int sns, Graph *g, int t):EmbeddingSpaceRGPM(s,sns,g) { 
	nthreads = t;
}

template <class T, class A>
EmbeddingSpaceRGPM<T,A>::EmbeddingSpaceRGPM(int s, int sns, Graph *g, std::string conf, std::string configString): EmbeddingSpaceRGPM(s,sns,g) {
	Config::load(conf,configString);
	Config::load(conf);
	Config::print();
	this->checkConfigFile();
	checkConfigFileRGPM();
}

template <class T, class A>
EmbeddingSpaceRGPM<T,A>::EmbeddingSpaceRGPM(int s, int sns, Graph *g, std::string conf, std::string configString, int t): EmbeddingSpaceRGPM(s,sns,g,conf,configString) {
	nthreads=t;
}

template <class T, class A>
void EmbeddingSpaceRGPM<T,A>::checkConfigFileRGPM() {
	if (Config::existKey(std::string("MAX_NUM_TOURS"))) 
		MAX_NUM_TOURS = Config::getKeyAsUint(std::string("MAX_NUM_TOURS"));
	if (Config::existKey(std::string("MAX_NUM_TOURS_BATCH"))) 
		MAX_NUM_TOURS_BATCH = Config::getKeyAsUint(std::string("MAX_NUM_TOURS_BATCH"));
	if (Config::existKey(std::string("MAX_TOUR_STEPS"))) 
		MAX_TOUR_STEPS = Config::getKeyAsInt(std::string("MAX_TOUR_STEPS"));
}

template <class T, class A>
void EmbeddingSpaceRGPM<T,A>::run_rw() {
	if (this->USE_PSRW_ESTIMATOR){
            // Since random walks need to be done on a smaller HON
	    this->size -= 1;
	}

	if (!this->init_rw()) {
		std::cout << "init fail!" << std::endl;
		return;
	}

	uint i = 0;
	while (i < this->MAX_RW_STEPS) {
		//create supernode
		SuperEmbedding<T> se = createBFSGroup(this->currEmbedding, snSize);
		LOG(debug) << se;

		//get estimates
		TourStatsWords tourStats = getGroupStats(this->currEmbedding, se, MAX_NUM_TOURS);

		if (!tourStats.ret) {
			std::cout << "Problem detected! over tour! " << this->currEmbedding << std::endl;
			exit(1);
		}

		// update global hashmap
		//for (std::pair<earray<uint>, float128> patternPair : tourStats.ns) {
		for (std::pair<earray<uint>*, float128> patternPair : tourStats.ns) {
			//std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator itPattern = this->total_counts_estimate.find(patternPair.first);
			std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(patternPair.first);
			if (itPattern == this->total_counts_estimate.end()) {
                		this->total_counts_estimate.insert(std::make_pair(patternPair.first, patternPair.second));
			} else {
				itPattern->second += patternPair.second;
			}
		}

		std::pair<Mod,bool> mod = this->getNextRandomModification(this->currEmbedding);
		this->currEmbedding.replaceWord(mod.first.rmId,mod.first.addId);
		
		i++;
		if (i%1000==0) std::cout << i << " embeddings were produced! " << std::endl;

	}

	//for (std::pair<earray<uint>, float128> patternPair: this->total_counts_estimate) {
	for (std::pair<earray<uint>*, float128> patternPair: this->total_counts_estimate) {
		if (this->USE_PSRW_ESTIMATOR)  {
		        //patternPair.second=REDUCTION_FACTOR*(patternPair.second*this->getPSRWBias(*patternPair.first, this->size+1));
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size+1, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		else  {
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		this->total_count_estimate += patternPair.second;
	}
	
	std::cout << "total final count estimate: " << std::fixed << this->total_count_estimate << " or " << std::scientific << this->total_count_estimate << std::endl;

} 

template <class T, class A>
TourStatsWords EmbeddingSpaceRGPM<T,A>::getGroupStats(T &embedding, SuperEmbedding<T> &se, int nTours) {
	LOG(debug) << "getting group stats for: " << embedding;
	if (se.isEmpty()) {
		std::cout << "SuperNode is empty!" << std::endl;
		exit(1);
	}
	//create pool of stats, one for each thread.
	for (int i = 0; i < this->nthreads; i++) tourStatsThreads.push_back(TourStatsWords());

	double external = se.getExternalDegree();
	std::cout << "external degree: " << external << std::endl;

	TourStatsWords tourStats;
        tourStats = groupEstimateAllTours(embedding, se, nTours);

	//for (std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator it = tourStats.ns.begin(); it != tourStats.ns.end(); it++) {
	for (std::unordered_map<earray<uint>*, float128>::iterator it = tourStats.ns.begin(); it != tourStats.ns.end(); it++) {
		it->second = external / this->MAX_NUM_TOURS * it->second;
		//std::cout << "==>update previous pattern " << (it->first) << " count estimate: " << std::fixed << it->second << std::endl;
	}

	//pass through patterns and update values
	std::vector<T> &embs = se.getEmbeddings();
	for (int i = 0; i < (int) embs.size(); i++) {
		T embeddingCopy = embs[i];
		if (!this->USE_PSRW_ESTIMATOR) {
			tourStats.n+=1.;
			tourStats.insert(this->getPatternHash(embeddingCopy), 1.);
		}
		else {
			std::vector<Mod> mods = this->computeAllEmbeddingNeighborhood(embeddingCopy);
			for (int j = 0; j < (int) mods.size(); j++) {
				embeddingCopy.replaceWord(mods[j].rmId, mods[j].addId);
				double bias = this->getPSRWBias(embeddingCopy, mods[j].rmId);
				//double bias = 1./REDUCTION_FACTOR;
				if (se.hasEmbedding(embeddingCopy)) bias *= 0.5;
				tourStats.n += bias;
				tourStats.insert(this->getPatternHash(embeddingCopy, mods[j].rmId), bias);
				embeddingCopy.replaceWord(mods[j].addId, mods[j].rmId);
			}
		}
	}	
	return tourStats;
}

template <class T, class A>
TourStatsWords EmbeddingSpaceRGPM<T,A>::groupEstimateAllTours(T &embedding, SuperEmbedding<T> &se, int nTours) {
	tbb::task_scheduler_init init(this->nthreads);
	
	//groups
	TourStatsWords finalTourStats;

	if (se.getHasAll())
		return finalTourStats;

        tbb::parallel_for(
                        tbb::blocked_range<int>(0, nTours),
                        [&](const tbb::blocked_range<int> &r)  {

                        int id = tbb::this_task_arena::current_thread_index();
                        //std::cout << "using thread id " << id << " number of tourStats " << tourStatsThreads.size() <<  " range " << r.end()-r.begin() << std::endl;
                        for (int i = r.begin(); i < r.end(); i++)
                        this->groupEstimateUsingTour(embedding, se, i, tourStatsThreads[id]);

                        });

        for (TourStatsWords tourStats : tourStatsThreads) finalTourStats+=tourStats;


	std::cout << "FINISHTOUR " <<  finalTourStats << " REJECTION COUNTERS: " << this->REJECTION_COUNTER1 << "/" << this->REJECTION_COUNTER2 << std::endl;
	return finalTourStats;
}

template <class T, class A>
TourStatsWords &EmbeddingSpaceRGPM<T,A>::groupEstimateUsingTour(T embedding, SuperEmbedding<T> &se, int tourId, TourStatsWords &tourStats) {
	//std::cout << "processing tour id: " << tourId << " embedding " << embedding << std::endl;
	std::pair<T, bool> embeddingSN = se.getBiasedEmbeddingWithExternalDegree();
	if (!embeddingSN.second) {
		std::cout << "Warning: impossible to find a neighbor of SN Group!" <<std::endl;
		tourStats.ret = true;	
		return tourStats;
	}
	int steps = 1;

	ModSet mods = getModificationsOutSN(embeddingSN.first, se);

	std::pair<Mod, bool> mod;
	mod.first = *(Randness::getInstance()->random_element(mods.begin(), mods.end())); 

	//get embedding out of supernode
	T embeddingCopy = embeddingSN.first;
	embeddingCopy.replaceWord(mod.first.rmId,mod.first.addId);

	bool reachedSupernode = false;
	do {
		//get next modification in advance
		mod = this->getNextRandomModification(embeddingCopy);

		//std::cout << "rw embedding: " << embeddingCopy << std::endl;
		if (!this->USE_PSRW_ESTIMATOR) {
			tourStats.n+=1./mod.first.totalWeight;
			tourStats.insert(this->getPatternHash(embeddingCopy), 1./mod.first.totalWeight);
		}

		//walk to the next embedding
		embeddingCopy.replaceWord(mod.first.rmId, mod.first.addId);

		//check if the tour is in super node
		reachedSupernode = se.hasEmbedding(embeddingCopy);
		//if (reachedSupernode) std::cout << "reach supernode: " << embeddingCopy <<  std::endl;

		if (!reachedSupernode && this->USE_PSRW_ESTIMATOR) {
			// Edges incident on the supernode have been accounted for already
			//double bias = 1./REDUCTION_FACTOR;
			double bias = this->getPSRWBias(embeddingCopy, mod.first.rmId);
			tourStats.n += bias*0.5;
			tourStats.insert(this->getPatternHash(embeddingCopy, mod.first.rmId), bias*0.5);
		}

		steps++;

		//if (stepsRW%100==0) std::cout << "warning: tour steps " << steps  << " stepsRW " << stepsRW << " stepsNext " << stepsNext << std::endl;
	} while (steps < this->MAX_TOUR_STEPS && !reachedSupernode);

	tourStats.steps += steps;
	tourStats.stepsSecMoment+=std::pow((double)steps, 2)/REDUCTION_FACTOR;
	tourStats.ret = (steps < this->MAX_TOUR_STEPS) && tourStats.ret;

	return tourStats;	
}

template <class T, class A>
SuperEmbedding<T> EmbeddingSpaceRGPM<T,A>::createBFSGroup(T &embedding, int size) {
	SuperEmbedding<T> se;
	se.setQueueMaxSize(size);

	T embeddingCopy(this->g);

	if (size == 1) { 
		embeddingCopy = embedding;

		embeddingCopy.setDegree(this->getEmbeddingDegree(embeddingCopy));
		se.insertEmbeddingQueue(embeddingCopy);
	}
	else if (size > 1) {
		std::queue<T> queue;

		embeddingCopy = embedding;

		embeddingCopy.setDegree(this->getEmbeddingDegree(embeddingCopy));
		se.insertEmbeddingQueue(embeddingCopy);
		queue.push(embeddingCopy);
		std::cout << "inserting : " << embeddingCopy << " degree: " << this->getEmbeddingDegree(embeddingCopy) << std::endl;

		int attempt=0;

		bool hasAttempt = attempt<std::max(this->MAX_INIT_ATTEMPT,size);	
		while (se.getNumberOfEmbeddings() < size && hasAttempt && !queue.empty()) {
			embeddingCopy = queue.front();
			std::vector<Mod> mods = this->computeAllEmbeddingNeighborhood(embeddingCopy);
			for (int j = 0; j < (int) mods.size(); j++) {
				embeddingCopy.replaceWord(mods[j].rmId,mods[j].addId);
				//put the embedding in the SE if it is not there already
				if (!se.hasEmbedding(embeddingCopy)) {
					embeddingCopy.setDegree(this->getEmbeddingDegree(embeddingCopy));
					se.insertEmbeddingQueue(embeddingCopy);
					queue.push(embeddingCopy);
					std::cout << "inserting : " << embeddingCopy << " degree: " << this->getEmbeddingDegree(embeddingCopy) << std::endl;
				}

				//return the embedding to its original form
				embeddingCopy.replaceWord(mods[j].addId,mods[j].rmId);
				attempt++;
				hasAttempt = attempt<std::max(this->MAX_INIT_ATTEMPT,size);
				if (!hasAttempt || se.getNumberOfEmbeddings() >= size) break;
			}
			queue.pop();

		}
	}

	//std::cout << "computing supernode external and internal degrees..." << std::endl;

	//Update embeddings
	se.updateEmbeddingCodeIndex();
	se.updateDegrees();

	std::cout << "SuperNode Size: " << se.getNumberOfEmbeddings() << " External Degree: " << se.getExternalDegree() << " Internal Degree: " << se.getInternalDegree()  << " Has all: " << se.getHasAll() << std::endl;

	return se;
}

template <class T, class A>   
ModSet EmbeddingSpaceRGPM<T,A>::getModificationsToSN(T &e, SuperEmbedding<T> &se) {
	ModSet mods;
	std::vector<Mod> allMods = this->computeAllEmbeddingNeighborhood(e);
	for (Mod mod : allMods) {
		e.replaceWord(mod.rmId,mod.addId);
		if (se.hasEmbedding(e)) mods.insert(mod);
		e.replaceWord(mod.addId,mod.rmId);
	}
	return mods;
}

template <class T, class A>   
ModSet EmbeddingSpaceRGPM<T,A>::getModificationsOutSN(T &e, SuperEmbedding<T> &se) {
	ModSet mods;
	std::vector<Mod> allMods = this->computeAllEmbeddingNeighborhood(e);
	for (Mod mod : allMods) {
		e.replaceWord(mod.rmId,mod.addId);
		if (!se.hasEmbedding(e)) mods.insert(mod);
		e.replaceWord(mod.addId,mod.rmId);
	}
	return mods;
}

template class EmbeddingSpaceRGPM<VertexInducedEmbedding, AggregatorPatternCounter>;
