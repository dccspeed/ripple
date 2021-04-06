#include "EmbeddingUtils.h"
#include <algorithm>

/*	thread_local int EmbeddingUtils::disc[MAX_EMBEDDING_SIZE] = {-1};
	thread_local int EmbeddingUtils::low[MAX_EMBEDDING_SIZE] = {-1};
	thread_local int  EmbeddingUtils::parent[MAX_EMBEDDING_SIZE+1] = {-1};
	thread_local uint EmbeddingUtils::conns[MAX_EMBEDDING_SIZE+1] = {0};
   */


/*bool EmbeddingUtils::isQuasiClique(BasicEmbedding &embedding, double a) {

	std::vector<int> &words = embedding.getWords();
	double factor = a*(double)(embedding.getNumWords() - 1.);

	for (int u = 0; u < embedding.getNumWords(); ++u) {
		int d = embedding.getWordDegree(words[u]);
		//std::cout << "factor : " << factor << " degree node: " << d << std::endl;
		if (factor > d) return false;
	}
	return true;
}*/
