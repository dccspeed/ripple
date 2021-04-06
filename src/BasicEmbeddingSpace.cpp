#include "BasicEmbeddingSpace.h"
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
EmbeddingSpace<T,A>::EmbeddingSpace(int s, Graph *g): currEmbedding(g), INITIAL_EMBEDDING(g) {  
	size=s;
	this->g = g;
	snSize = 1;
	setStackSize();
	createSpecificPhashes(10);
};

template <class T, class A>
EmbeddingSpace<T,A>::EmbeddingSpace(int s, Graph *g, std::string conf, std::string configString):EmbeddingSpace(s,g) {
	Config::load(conf,configString);
	Config::load(conf);
	Config::print();
	checkConfigFile();
}
	
template <class T, class A>
void EmbeddingSpace<T,A>::checkConfigFile() {
	if (Config::existKey(std::string("MAX_RW_STEPS"))) 
		MAX_RW_STEPS = Config::getKeyAsUint(std::string("MAX_RW_STEPS"));
	if (Config::existKey(std::string("INIT_RW_STEPS"))) 
		INIT_RW_STEPS = Config::getKeyAsInt(std::string("INIT_RW_STEPS"));
	if (Config::existKey(std::string("MAX_INIT_ATTEMPT"))) 
		MAX_INIT_ATTEMPT = Config::getKeyAsInt(std::string("MAX_INIT_ATTEMPT"));
	if (Config::existKey(std::string("INITIAL_EMBEDDING"))) { 
		std::string INITIAL_EMBEDDING_STRING = Config::getKeyAsString(std::string("INITIAL_EMBEDDING"));
		INITIAL_EMBEDDING.loadFromString(INITIAL_EMBEDDING_STRING);
	}
	if (Config::existKey(std::string("USE_PSRW_ESTIMATOR"))) {
		USE_PSRW_ESTIMATOR = Config::getKeyAsBoolean(std::string("USE_PSRW_ESTIMATOR"));
	}
	if (Config::existKey(std::string("OUTPUT_EMB_FILE"))) {
		std::string filename = Config::getKeyAsString(std::string("OUTPUT_EMB_FILE"));
		OUTPUT_EMB_FILE.open(filename, std::ofstream::out); 
		if (!OUTPUT_EMB_FILE.is_open()){ 
			std::cout << "error: cant not open output embeddings file! " << filename <<  std::endl;
			exit(1);
		}
	}
    	if (Config::existKey(std::string("TYPE_COUNT"))) {
    	  TYPE_COUNT = Config::getKeyAsInt(std::string("TYPE_COUNT"));
    	}
}

template <class T, class A>
bool EmbeddingSpace<T,A>::getValidEmbedding() {
	currEmbedding.reset();
	
	//get init node
	int n = Randness::getInstance()->get_a_random_number(0, currEmbedding.getTotalNumWords());
	std::cout << "init rw : first node " << n << " degree: " << g->getDegreeOfNodeAt(n) <<  std::endl;
	currEmbedding.addWord(n);

	std::unordered_set<int> expansions = currEmbedding.getValidElementsForExpansion();

	if (expansions.empty()) {
		std::cout << "expansions empty!" << std::endl;
	}

	while (!expansions.empty() && currEmbedding.getNumWords() < size) {
		int n = Randness::getInstance()->get_a_random_number(0, expansions.size());

		//get element 
		std::unordered_set<int>::iterator it = expansions.begin();
		for (int k = 0; k < n; k++)
			it++;

		std::cout << "init rw : new word " << *it << " degree: " << g->getDegreeOfNodeAt(*it) << std::endl;
		currEmbedding.addWord(*it);
		expansions = currEmbedding.getValidElementsForExpansion();
		if (expansions.empty()) {
			std::cout << "expansions empty!" << std::endl;
		}
	}
	currEmbedding.print();
	std::cout << "num words " << currEmbedding.getNumWords() << " size " << size << std::endl;
	return currEmbedding.getNumWords() == size;
}


template <class T, class A>
bool EmbeddingSpace<T,A>::init_rw() {
	std::cout << "init rw" << std::endl;
	currEmbedding.reset();
	bool r = false;

	if (!INITIAL_EMBEDDING.isEmpty()) {
		currEmbedding = INITIAL_EMBEDDING;
		return true;
	} 
	else {
		for(int i = 0; i < MAX_INIT_ATTEMPT; i++) {
			if (getValidEmbedding()) {
				r = true;
				break;
			}
		}
	}

	// refine the initial embedding
	if (r) {
		for(int i = 0; i < INIT_RW_STEPS; i++) {
			if (i%10000==0) std::cout << "init rw step: " << i << std::endl;
			std::pair<Mod,bool> mod = getNextRandomModification(currEmbedding);
			if (!mod.second) {
				std::cout << "warning: no neighbors were found! aborting..." << std::endl;
				exit(1);
			}
			currEmbedding.replaceWord(mod.first.rmId,mod.first.addId);
		}
	}

	return r;
}

template <class T, class A>
void EmbeddingSpace<T,A>::run_exact_rec(T &e, int wordId) {
	e.addWord(wordId);
	if (e.getNumWords()==size) {
		total_count_estimate++;
		//	std::cout << total_count_estimate << " embeddings were produced! " << std::endl;
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
		std::vector<Mod> mods = computeAllEmbeddingNeighborhood(e);
		for (int i = 0; i < (int) mods.size(); i++) {
			e.replaceWord(mods[i].rmId, mods[i].addId);
			if (this->USE_PSRW_ESTIMATOR){
				double bias = this->getPSRWBias(e, mods[i].rmId);
				//double bias = 1./REDUCTION_FACTOR;
				bias *= 0.5;
				earray<uint> *phash = this->getPatternHash(e, mods[i].rmId);

				//std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator itPattern = this->total_counts_estimate.find(phash);
				std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(phash);
				if (itPattern == this->total_counts_estimate.end()) {
					this->total_counts_estimate.insert(std::make_pair(phash, (float128) bias));
				} else {
					itPattern->second += bias;
				}
			}
			e.replaceWord(mods[i].addId, mods[i].rmId);
		}
	}
	else {
		std::unordered_set<int> expansions = e.getValidElementsForExpansion();
		for (std::unordered_set<int>::iterator it = expansions.begin(); it!= expansions.end(); it++) {	
			if (e.isCanonicalEmbeddingWithWord(*it)) {
				run_exact_rec(currEmbedding, *it);
			}
		}
	}
	e.removeLastWord();		
}

template <class T, class A>
void EmbeddingSpace<T,A>::run_exact() {
	if (this->USE_PSRW_ESTIMATOR){
		// Since random walks need to be done on a smaller HON
		this->size -= 1;
	}

	currEmbedding.reset();
	int numWords = currEmbedding.getTotalNumWords();
	for (int i = 0; i < numWords; i++) {
		run_exact_rec(currEmbedding, i);
	}

	//for (std::pair<earray<uint>, float128> patternPair: total_counts_estimate) {
	for (std::pair<earray<uint>*, float128> patternPair: total_counts_estimate) {
		if (this->USE_PSRW_ESTIMATOR) {
			//patternPair.second=REDUCTION_FACTOR*(patternPair.second*this->getPSRWBias(*patternPair.first, this->size+1));
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size+1, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
		else {
			std::cout << "==>update final pattern " << Canonical::getMotivoHash(patternPair.first, this->size, 30) << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
		}
	}
	
	std::cout << "total final count estimate: " << std::fixed << this->total_count_estimate << " or " << std::scientific << this->total_count_estimate << std::endl;

} 

template <class T, class A>
void EmbeddingSpace<T,A>::run_rw() {
	if (this->USE_PSRW_ESTIMATOR){
		// Since random walks need to be done on a smaller HON
		this->size -= 1;
	}

	if (!init_rw()) {
		std::cout << "init fail!" << std::endl;
		return;
	}

	uint i = 0;
	double bias = 0;
	earray<uint> *phash;
	while (i < MAX_RW_STEPS) {
		std::pair<Mod,bool> mod = getNextRandomModification(currEmbedding);

		if (!this->USE_PSRW_ESTIMATOR) {
			bias = 1./mod.first.totalWeight;
			phash = this->getPatternHash(currEmbedding);

		}

		currEmbedding.replaceWord(mod.first.rmId,mod.first.addId);

		if (this->USE_PSRW_ESTIMATOR) {
			bias = this->getPSRWBias(currEmbedding, mod.first.rmId);
			//bias = 1./REDUCTION_FACTOR;
			phash = this->getPatternHash(currEmbedding, mod.first.rmId);
		}

		//update counters
		//std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator itPattern = this->total_counts_estimate.find(phash);
		std::unordered_map<earray<uint>*, float128>::iterator itPattern = this->total_counts_estimate.find(phash);
		if (itPattern == this->total_counts_estimate.end()) {
			this->total_counts_estimate.insert(std::make_pair(phash, (float128) bias));
		} else {
			itPattern->second += bias;
		}

		i++;
		if (i%1000==0) {
			std::cout << i << " embeddings were produced! " << std::endl;
		}
	}

	std::cout.setf(std::ios::fixed);

	//get the total value to normalize
	for (std::unordered_map<earray<uint>*, float128>::iterator it = total_counts_estimate.begin(); it != total_counts_estimate.end(); it++) {
		//if (this->USE_PSRW_ESTIMATOR)  it->second=REDUCTION_FACTOR*(it->second*this->getPSRWBias(*it->first, this->size+1));
		total_count_estimate+=it->second;
	}
	
	for (std::pair<earray<uint>*, float128> patternPair: total_counts_estimate) {
		//get code according to rw type
		std::string code;
		if (this->USE_PSRW_ESTIMATOR) code = Canonical::getMotivoHash(patternPair.first, this->size+1, 30);
		else  code = Canonical::getMotivoHash(patternPair.first, this->size, 30);
		
		patternPair.second/=total_count_estimate;
		std::cout << "==>update final pattern " << code << " count estimate: " << std::fixed << patternPair.second << " or " << std::scientific << patternPair.second << std::endl;
	}

} 

template <class T, class A>
double EmbeddingSpace<T,A>::getEmbeddingDegree(T &e, ModSet &mods) {
	std::pair<Mod, bool> r = computeEmbeddingNeighborhood(e,mods);
	if (!r.second) {
		std::cout << "warning: no neighbors were found!" << std::endl;	
	}
	return r.first.totalWeight;
}

template <class T, class A>
double EmbeddingSpace<T,A>::getEmbeddingDegree(T &e) {
	ModSet empty_modset;
	return getEmbeddingDegree(e, empty_modset);
}

template <class T, class A>
std::pair<Mod,bool> EmbeddingSpace<T,A>::getNextRandomModification(T &e, ModSet &mods) {
	std::pair<Mod,bool> mod;

	//try to get by rejection
	if (this->USE_PSRW_ESTIMATOR) {
		mod = getNextRandomModificationByRejection(e, mods);
	}
	else {
		mod = computeEmbeddingNeighborhood(e, mods);
	}

	if (!mod.second) std::cout << "warning: there are no embedding neighbors!" << std::endl;

	return mod;
}

template <class T, class A>
std::pair<Mod,bool> EmbeddingSpace<T,A>::getNextRandomModification(T &e) {
	ModSet empty_modset = ModSet({});
	return getNextRandomModification(e, empty_modset);
}

//template class EmbeddingSpace<EdgeInducedEmbedding, AggregatorPatternCounter>;
template class EmbeddingSpace<VertexInducedEmbedding, AggregatorPatternCounter>;
