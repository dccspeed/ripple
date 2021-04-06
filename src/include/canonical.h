#ifndef CANONICAL_H
#define CANONICAL_H

#include <stdlib.h>
#include <unordered_map>
#include <utils.hh>
#include <graph.hh>
#include <bliss_C.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_set.h>
#include "utils.h"
#include "BasicEmbedding.h"
#include "graph.h"


class Canonical {
	public:
   		//thread_local static bliss::Graph bg;
		static tbb::atomic<int> hits;
		static tbb::atomic<int> misses;
		static tbb::concurrent_hash_map<size_t, size_t> pattern_to_canonical; 
		static tbb::concurrent_hash_map<earray<uint>, earray<uint>*, earrayhash<uint>> pattern_to_canonical2;
		static tbb::concurrent_unordered_set<earray<uint>, earrayhash<uint>> canonical2;
                static earray<uint> nullphash;


		//static size_t getHashScratch(bliss::Graph &);
		static earray<uint> *getHash2(BasicEmbedding &);
		static earray<uint> *getHash2(BasicEmbedding &, int );
		static earray<uint> *getHash2(earray<uint>&, int);
		//static size_t getHash(BasicEmbedding &);
		//static size_t getHash(BasicEmbedding &, int);
		//static size_t getHash(Graph &);
		//static size_t getHash(bliss::Graph &);
		//static unsigned int *getPermutation(bliss::Graph &);
		
		//static void report_aut(void*, const unsigned int, const unsigned int*);
		//static void get_automorphism_map(unsigned int *, const unsigned int N, const unsigned int* perm, const unsigned int offset);
		
		//static void merge_sets(int *, size_t );
		//static bliss::Graph getBlissGraph(BasicEmbedding &);
		//static bliss::Graph getBlissGraph(BasicEmbedding &, int);
		static bliss::Graph getBlissGraph(earray<uint>& , int);
		//static size_t getNaiveHash(BasicEmbedding &, int);
		//static size_t getNaiveHash(BasicEmbedding &);
		static earray<uint> *get_earray_hash(bliss::Graph *);
		static std::string getMotivoHash(earray<uint> *, int, int);

};

#endif
