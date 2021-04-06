#ifndef EDGEINDUCEDEMBEDDING_H
#define EDGEINDUCEDEMBEDDING_H

#include "BasicEmbedding.h"
#include "graph.h"
#include "random.h"
#include <set>

class EdgeInducedEmbedding: public BasicEmbedding {

public: 
   /*std::vector<int> edges;
   std::vector<int> vertices; 
   std::unordered_set<int> edgesSet;
   std::unordered_map<int, int> verticesMap; 
   std::unordered_map<int,std::pair<int, uint>> neighborhoodMap;
   std::vector<int> degrees;*/

protected:
   bool areWordsNeighbours(int wordId1, int wordId2);

public :
    EdgeInducedEmbedding();
    EdgeInducedEmbedding(Graph *g);
    EdgeInducedEmbedding(Graph *g, std::vector<int>);
    void setFromEmbedding(EdgeInducedEmbedding other);
    void reset();
    
    std::vector<int> &getWords();
    std::unordered_map<int,int> &getExtensionsWords();
    int getNumWords();
    int getTotalNumWords();
    void addWord(int);
    void replaceWord(int, int);
    void removeWord(int);
    void removeLastWord();
    NeighborhoodSet getWordNeighbors(int);

    std::vector<int> getValidElementsForExpansionSorted();
    std::vector<int> &getValidElementsForExpansionSorted(int);
    std::unordered_set<int> getValidElementsForExpansionWith(std::unordered_set<int> &);
    std::unordered_map<int,uint> getValidElementsForExpansionWithConnectionPattern();
    std::unordered_set<int> getValidElementsForExpansion();
    NeighborhoodSet &getValidElementsForExpansion(int);
    std::unordered_set<int> getValidElementsForContraction();
    std::unordered_set<int> getValidElementsForContractionWithWord(int);
    
    int getRandomWordBiased();    
};

#endif
