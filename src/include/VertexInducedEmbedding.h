#ifndef VERTEXINDUCEDEMBEDDING_H
#define VERTEXINDUCEDEMBEDDING_H

#include "BasicEmbedding.h"
#include "graph.h"
#include "random.h"
#include <set>
#include <boost/container/flat_set.hpp>
#include <tbb/parallel_for.h>
class VertexInducedEmbedding: public BasicEmbedding {


public :
    VertexInducedEmbedding();
    VertexInducedEmbedding(Graph *g);
    VertexInducedEmbedding(Graph *g, earray<int> &, int);
    ~VertexInducedEmbedding();
    void setFromEmbedding(VertexInducedEmbedding other);
    earray<int> &getWords();
    int getNumWords();
    int getTotalNumWords();
    void addWord(int);
    void replaceWord(int, int);
    void removeWord(int);
    void removeLastWord();
    NeighborhoodSet getWordNeighbors(int);
    bool hasWord(int);
    
    std::vector<int> getValidElementsForExpansionSorted();
    std::vector<int> &getValidElementsForExpansionSorted(int);
    std::unordered_set<int> getValidElementsForExpansion();
    std::unordered_map<int,uint> getValidElementsForExpansionWithConnectionPattern();
    std::unordered_set<int> getValidElementsForExpansionWith(std::unordered_set<int> &);
    NeighborhoodSet &getValidElementsForExpansion(int);
    std::unordered_set<int> getValidElementsForContraction();
    std::unordered_set<int> getValidElementsForContractionWithWord(int);
    

inline bool areWordsNeighbours(int wordId1, int wordId2) {
	return graph->isNeighbor(wordId1, wordId2);
}
    
int getRandomWordBiased(); 
    static void test(Graph &g);
};

#endif
