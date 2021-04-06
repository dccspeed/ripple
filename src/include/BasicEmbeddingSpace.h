#ifndef BASICEMBEDDINGSPACE_H
#define BASICEMBEDDINGSPACE_H

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
#include "TourStats.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <graph.hh>
#include <cache.h>
#include <fstream>


template <class T, class A> 
class EmbeddingSpace {
		
	public : 
		Graph *g;
		T currEmbedding;
		int size; // embedding space related size
		int snSize; // embedding space related size

		float128 total_count_estimate = 0;
		//std::unordered_map<earray<uint>, float128, earrayhash<uint>> total_counts_estimate;
		std::unordered_map<earray<uint>*, float128> total_counts_estimate;
                
		std::vector<earray<uint>*> specific_phashes;

		ModSet mods_empty;
		EmbeddingSpace(int , Graph *); 
		EmbeddingSpace(int , Graph *, std::string, std::string); 
		void checkConfigFile();
		
		virtual bool init_rw();
		void run_rw();
		void run_exact_rec(T &, int);
		void run_exact();

		bool getValidEmbedding();
		int getEmbeddingNumNeighbors(T &);
		int getEmbeddingNumNeighbors(T &, ModSet &);
		double getEmbeddingDegree(T &);
		double getEmbeddingDegree(T &, ModSet &);
		std::pair<Mod,bool> getNextRandomModification(T &);
		std::pair<Mod,bool> getNextRandomModification(T &, ModSet &);

		uint MAX_RW_STEPS=1000000;
		int INIT_RW_STEPS=1000000;
		int MAX_INIT_ATTEMPT=10000;
		int MAX_CACHE_SIZE=10000000;
		T INITIAL_EMBEDDING;
		std::ofstream OUTPUT_EMB_FILE;
		bool USE_PSRW_ESTIMATOR=false;
    		int TYPE_COUNT=0;
		uint128_t REJECTION_COUNTER1 = 0;
		uint128_t REJECTION_COUNTER2 = 0;
		double SECOND_MOMENT = 0;

		inline double getPSRWBias(T &e, int extraWord) {
			double vCount = e.getNumVertices() + 1 - bitCount(EmbeddingUtils::articulation(e, extraWord));
			double bias = 2. / (vCount * (vCount - 1));
			return bias;
		}

          	inline double getPSRWBias(T &e) {
                        double vCount = e.getNumVertices() + 1 - bitCount(EmbeddingUtils::articulation(e));
                        double bias = 2. / (vCount * (vCount - 1));
                        return bias;
                }

		inline double getPSRWBias(earray<uint> &connections, int k) {
                        double vCount = k - bitCount(EmbeddingUtils::articulation(connections, k));
                        double bias = 2. / (vCount * (vCount - 1));
                        return bias;
		}

		virtual std::pair<Mod, bool> computeEmbeddingNeighborhood(T &e, ModSet &mods) {
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
					//get weight of mod
					double weight = 1.;
					double prob = Randness::getInstance()->random_uni01();
					if (prob <= (double) weight/(mod.totalWeight+weight)) {
						mod.addId = *it;
						mod.rmId = *it2;
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

		virtual std::vector<Mod> computeAllEmbeddingNeighborhood(T &e) {
			std::unordered_map<size_t, std::unordered_set<int>> contractionMap;
			std::vector<Mod> mods;

			std::unordered_set<int> expansions = e.getValidElementsForExpansion();
			if (expansions.empty()) {	
				std::cout << "impossible to walk! ";
				std::cout << "num possible exp. equal to " << expansions.size() << std::endl;
				exit(1);
			}

			double totalWeight = 0;
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
					//get weight of mod
					double weight = 1.;
					totalWeight+=weight;
					mods.push_back(Mod(*it2,*it));
				}
			}

			if (totalWeight == 0) {
				std::cout << "error: there is no valid possible expansion!" << std::endl;
				exit(1);
			}

			for (uint i = 0; i < mods.size(); i++) {
				mods[i].totalWeight = totalWeight;
			}

			return mods;
		}

		virtual std::pair<Mod,bool> getNextRandomModificationByRejection(T &e, ModSet &mods) {
			//std::cout << "getNextRandomModificationByRejection" << e << std::endl; 
			std::pair<Mod, bool> mod;
			mod.second = false;

			earray<int> &words = e.getWords();
			//uint contraction2 = 0;
			//std::vector<bool> articulation2 = EmbeddingUtils::articulation(e);
			//for (int i = 0; i < (int) words.size(); i++)
			//if (!articulation2[i]) setKthBit(&contraction2, i);

			//uint articulation = e.articulation();
			uint articulation = EmbeddingUtils::articulation(e);
			int att = 0;
			do {
				att++;
				//select node to be anchor
				int totalWeight=0;
				uint anchorIdx=0;
				for (int i = 0; i < e.getNumWords(); i++) {
					int weight = g->getDegreeOfNodeAt(words[i]);

					double prob = Randness::getInstance()->random_uni01();
					if (prob <= (double) weight/(totalWeight+weight)) anchorIdx=i; 
					totalWeight+=weight;
				}

				//select a node to get in	
				mod.first.addId = g->getRandomNodeNeighbor(words[anchorIdx]);
				int bias = 0;
				for (int i = 0; i < e.getNumWords(); i++) if (g->isNeighbor(words[i], mod.first.addId)) bias++;
				double prob = Randness::getInstance()->random_uni01();

				if (!e.hasWord(mod.first.addId) && prob<=1./(double)bias) {	
					//get the node to leave
					int leaveIdx = Randness::getInstance()->get_a_random_number(0, e.getNumWords());
					mod.first.rmId = words[leaveIdx];
					if (bias==1 && g->isNeighbor(mod.first.addId, mod.first.rmId)) {
						mod.second = false;	
					}
					else if (!isKthBitSet(articulation,leaveIdx)) {
						mod.second = true;	
					}
					else {
						REJECTION_COUNTER2++;
						uint articulation2 = EmbeddingUtils::articulation(e,mod.first.addId);
						if (!isKthBitSet(articulation2,leaveIdx)) mod.second = true;
					}
				}
			} while (!mod.second && att < MAX_INIT_ATTEMPT);
			REJECTION_COUNTER1+=att;
			if (!mod.second) {
				std::cout << "warning: getNextRandomModificationByRejection fail. " << e << std::endl; 
				exit(1);
			}
			return mod;
		}



		virtual earray<uint> *getPatternHash(T &e, int wordId) {
			if (TYPE_COUNT==1) {
				earray<uint> connections = e.getConnections();
				uint k = e.getNumVertices();
				if (wordId>=0) {
					earray<int> &vertices = e.getVertices();
					for (uint i = 0; i < k; i++) {
						if (e.areWordsNeighbours(vertices[i], wordId)) {
							setKthBit(&(connections[i]),k);
							setKthBit(&(connections[k]),i);
						}
					}
					k++;
				}
				earray<uint> degreeDist;
				for (uint i = 0; i < k; i++) degreeDist[bitCount(connections[i])]++;
				if ((degreeDist[1]==2 && degreeDist[2]==k-2) ||
						(degreeDist[2]==k) || 
						(degreeDist[1]==k-1 && degreeDist[k-1]==1) ||
						(degreeDist[k-1]==k)) {
					return Canonical::getHash2(e, wordId);
				}
				else {
					return &Canonical::nullphash;
				}

			}
			else if (TYPE_COUNT==2) {
				earray<uint> connections = e.getConnections();
				uint k = e.getNumVertices();
				double min_factor = 1.;
				if (wordId>=0) {
					earray<int> &vertices = e.getVertices();
					for (uint i = 0; i < k; i++) {
						if (e.areWordsNeighbours(vertices[i], wordId)) {
							setKthBit(&(connections[i]),k);
							setKthBit(&(connections[k]),i);
						}
						double factor = (double) bitCount(connections[i]) /(k);
						if (min_factor > factor) min_factor = factor;
					}
					double factor = (double) bitCount(connections[k]) /(k);
					if (min_factor > factor) min_factor = factor;
					k++;

					/*earray<uint> degreeDist;
					for (uint i = 0; i < k; i++) degreeDist[bitCount(connections[i])]++;
					if (degreeDist[1]==2 && degreeDist[2]==k-2 && min_factor != 1./3)  {
						std::cout << "path! " << min_factor << std::endl;
						exit(1);
					}
					else if (degreeDist[2]==k && min_factor != 2./3) {
						std::cout << "cicle! " << min_factor << std::endl;
						exit(1);
					}
					else if (degreeDist[1]==k-1 && degreeDist[k-1]==1 && min_factor!= 1./3) {
						std::cout << "star! " << min_factor << std::endl;
						exit(1);
					}
					else if (degreeDist[k-1]==k && min_factor!=1) {
						std::cout << "clique! " << min_factor << std::endl;
						exit(1);
					}
					else if (degreeDist[2]==2 && degreeDist[3]==2 && min_factor != 2./3) {
						std::cout << "other1! " << min_factor << std::endl;
						exit(1);
					}
					else if (degreeDist[1]==1 && degreeDist[2]==2 && degreeDist[3]==1 && min_factor != 1./3) {
						std::cout << "other2! " << min_factor << std::endl;
						exit(1);
					}*/

				}
				else {
					min_factor = e.quasiCliqueScore();
				}

				return specific_phashes[getPhashDensityIdx(min_factor)];
			}

			else return Canonical::getHash2(e, wordId);
		}

		virtual earray<uint> *getPatternHash(T &e) {
			return this->getPatternHash(e, -1);
		}

		int getPhashDensityIdx(double a) {
			uint idx = 0;
			if (a>=0 && a<0.25) {
				idx = 0;
			}
			else if (a>=0.25 && a < 0.5) {
				idx = 1;
			}
			else if (a>=0.5 && a < 0.75) {
				idx = 2;
			}
			else if (a>=0.75 && a <= 1) {
				idx = 3;
			}	
			else {
				std::cout << "error: wrong density value!" << std::endl;
				exit(1);
			}
			return idx;
		}

		void createSpecificPhashes(int n) {
			for (int i = 0; i < this->size; ++i) {
				earray<uint> *con = new earray<uint>();
				(*con)[i] = ~(0);
				std::cout << "con: " << *con << std::endl;
				specific_phashes.push_back(con);
				std::cout << "motivo hash: " << Canonical::getMotivoHash(con, this->size, 30) << std::endl;
			}

			for (int i = 0; i < this->size-1; ++i) {
				earray<uint> *con = new earray<uint>();
				(*con)[i] = ~(0);
				(*con)[i+1] = ~(0);
				std::cout << "con: " << *con << std::endl;
				specific_phashes.push_back(con);
				std::cout << "motivo hash: " << Canonical::getMotivoHash(con, this->size, 30) << std::endl;
			}

		}


};


#endif

