#ifndef EMBEDDINGUTILS_H
#define EMBEDDINGUTILS_H

#include "BasicEmbedding.h"
#include "graph.h"
#include <algorithm>
  

class EmbeddingUtils {
   private :
/*	thread_local static int disc[MAX_EMBEDDING_SIZE];
	thread_local static int low[MAX_EMBEDDING_SIZE];
	thread_local static int parent[MAX_EMBEDDING_SIZE+1];
	thread_local static uint conns[MAX_EMBEDDING_SIZE+1];
	*/
   
public: 

	static inline void articulationRec(int u, uint size, uint *ap, uint *visited, int *disc, int *low, int *parent, uint *conns, int *time) {
		// number of children in the DFS
		int children = 0;

		setKthBit(visited, u);
		int numNeighbors = bitCount(conns[u]);

		// discovery time and low value
		disc[u] = ++(*time);
		low[u] = *time;

		int k=0;
		while (k != (int) size && numNeighbors > 0) {
			if (!isKthBitSet(conns[u],k)) { 
				k++;
				continue;
			}
			if (!isKthBitSet(*visited, k)) {
				children++;
				parent[k] = u;
				articulationRec(k, size, ap, visited, disc, low, parent, conns, time);

				// Check if the subtree rooted with k has a connection to
				// one of the ancestors of u
				low[u] = std::min(low[u], low[k]);

				// u is an articulation point in following cases
				// (1) u is root of DFS tree and has two or more chilren.
				if (parent[u] == -1 && children > 1) setKthBit(ap,u);

				// (2) If u is not root and low value of one of its child
				// is more than discovery value of u.
				if (parent[u] != -1 && low[k] >= disc[u]) setKthBit(ap,u);
			}
			// Update low value of u for parent function calls.
			else if (k != parent[u])
				low[u] = std::min(low[u], disc[k]);
			k++;
			numNeighbors--;
		}
	}

	static inline uint articulation(BasicEmbedding &e, int wordId) {
		earray<int> &vertices = e.getVertices();
		earray<uint> &connections = e.getConnections();
		int time = 0;
		uint visited = 0;
		uint ap = 0;
		
		int disc[MAX_EMBEDDING_SIZE];
		int low[MAX_EMBEDDING_SIZE];
		int parent[MAX_EMBEDDING_SIZE+1];
		uint conns[MAX_EMBEDDING_SIZE+1];

		memset(disc, -1, (e.getNumVertices()+1) * sizeof(int));
		memset(low, -1, (e.getNumVertices()+1) * sizeof(int));
		memset(parent, -1, (e.getNumVertices()+1) * sizeof(int));
		memset(conns, 0, (e.getNumVertices()+1) * sizeof(int));

		for (int i = 0; i < e.getNumVertices(); i++) {
			conns[i] = connections[i];
			if (e.areWordsNeighbours(vertices[i], wordId)) {
				setKthBit(&conns[i],e.getNumVertices());
				setKthBit(&conns[e.getNumVertices()], i);
			}
		}

		articulationRec(0, e.getNumVertices()+1, &ap, &visited, disc, low, parent, conns, &time);
		unsetKthBit(&ap,(int)e.getNumVertices());

		return ap;
	}


	static inline uint articulation(earray<uint> &connections, int k) {
		int time = 0;
		uint visited = 0;
		uint ap = 0;
		
		int disc[MAX_EMBEDDING_SIZE];
		int low[MAX_EMBEDDING_SIZE];
		int parent[MAX_EMBEDDING_SIZE+1];

		memset(disc, -1, k * sizeof(int));
		memset(low, -1, k * sizeof(int));
		memset(parent, -1, k * sizeof(int));

		articulationRec(0, k, &ap, &visited, disc, low, parent, connections.data(), &time);

		return ap;
	}

	static inline uint articulation(BasicEmbedding &e) {
		return articulation(e.getConnections(), e.getNumVertices());
	}

	static inline	void dfsRec(BasicEmbedding &e, int u, uint *visited, int *parent, uint *conns) {

		setKthBit(visited, u);
		int numNeighbors = bitCount(conns[u]);

		int k=0;
		while (k != e.getNumVertices() && numNeighbors > 0) {
			if (!isKthBitSet(conns[u],k)) {
				k++;
				continue;
			}
			if (!isKthBitSet(*visited, k)) {
				parent[k] = u;
				dfsRec(e, k, visited, parent, conns);

			}
			k++;
			numNeighbors--;
		}

	}


	static inline uint dfs(BasicEmbedding &e) {
		earray<uint> &connections = e.getConnections();
		uint visited = 0;

		int parent[MAX_EMBEDDING_SIZE+1];
		
		memset(parent, -1, e.getNumVertices() * sizeof(int));
		dfsRec(e, 0,  &visited, parent, connections.data());

		return visited;
	}
};


#endif
