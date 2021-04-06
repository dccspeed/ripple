#include "VertexInducedEmbedding.h"
#include "EmbeddingUtils.h"
#include <graph.hh>
#include <algorithm>

VertexInducedEmbedding::VertexInducedEmbedding():BasicEmbedding() { 
};

VertexInducedEmbedding::VertexInducedEmbedding(Graph *g, earray<int> &ws, int s):BasicEmbedding(g, ws, s) { 
	for (int i = 0; i < s; i++) 
		addWord(ws[i]);
};

VertexInducedEmbedding::VertexInducedEmbedding(Graph *g):BasicEmbedding(g) { 
};

VertexInducedEmbedding::~VertexInducedEmbedding() { 
};

void VertexInducedEmbedding::setFromEmbedding(VertexInducedEmbedding other) {
}

NeighborhoodSet VertexInducedEmbedding::getWordNeighbors(int id) {
	assert(graph!=NULL);
	assert(id < graph->getNumberOfNodes());
        return graph->getNeighborhoodVertexOfVertexAt(id);
}

earray<int> &VertexInducedEmbedding::getWords() {
	return vertices2;
}

inline int VertexInducedEmbedding::getNumWords() {
	return getNumVertices();
}

inline bool VertexInducedEmbedding::hasWord(int word) {
	return hasVertex(word);
}

inline NeighborhoodSet &VertexInducedEmbedding::getValidElementsForExpansion(int vertexId) {
	return graph->getNeighborhoodVertexOfVertexAt(vertexId);
}

inline std::vector<int> &VertexInducedEmbedding::getValidElementsForExpansionSorted(int vertexId) {
	return graph->getNeighborhoodIdxVertexOfVertexAt(vertexId);
}

inline std::unordered_set<int> VertexInducedEmbedding::getValidElementsForContraction() {
	std::unordered_set<int> contractions;
	uint ap = EmbeddingUtils::articulation(*this);

	for (int i = 0 ; i < getNumVertices(); i++)
		if (!isKthBitSet(ap,i))
			contractions.insert(vertices2[i]);	
	return contractions;
}

inline std::unordered_set<int> VertexInducedEmbedding::getValidElementsForContractionWithWord(int wordId) {
	std::unordered_set<int> contractions;

	std::unordered_set<int> contractionsOld;
	uint ap = EmbeddingUtils::articulation(*this, wordId);
        for (int i = 0 ; i < getNumVertices(); i++)
              if (!isKthBitSet(ap, i) && wordId!=vertices2[i])
                     contractionsOld.insert(vertices2[i]); 

	return contractionsOld;
}


//it assumes that the neighbor vector is sorted
std::vector<int> VertexInducedEmbedding::getValidElementsForExpansionSorted() {
	std::vector<int> expansions;
	if (getNumVertices() == 0) {
		expansions.push_back(-1);
	}

	std::unordered_set<int> v(vertices2.begin(), vertices2.begin()+getNumVertices()); 
	std::vector<std::vector<int>::const_iterator> fwdIterators, endIterators;
	uint maxsize = 0;
	for (int i = 0; i < getNumVertices(); i++) {
		std::vector<int> &neighs = getValidElementsForExpansionSorted(vertices2[i]);
		//std::cout << "neighs " << i;
		//for (int j : neighs) 
		//	std::cout << " " << j;
		//std::cout << std::endl;
		fwdIterators.push_back(neighs.begin());
		endIterators.push_back(neighs.end());
		maxsize+=neighs.size();
	}
	expansions.reserve(maxsize);

	while (!fwdIterators.empty())
	{
		// Find out which iterator carries the smallest value
		size_t index = 0;
		for (size_t i = 1; i < fwdIterators.size(); ++i) {
			if (*fwdIterators[i] < *fwdIterators[index])
				index = i;
		}

		if (v.find(*fwdIterators[index])==v.end() && (expansions.empty() || expansions.back() < *fwdIterators[index])) {
			expansions.push_back(*fwdIterators[index]);
			//std::cout << "add " << *fwdIterators[index] << std::endl;
		}

		++fwdIterators[index];
		if (fwdIterators[index] == endIterators[index]) {
			fwdIterators.erase(fwdIterators.begin() + index);
			endIterators.erase(endIterators.begin() + index);
		}
	}

	//for (int i : expansions) 
	//	std::cout << " " << i;
	//std::cout << "size:" << expansions.size() << std::endl;
	
	return expansions;
}

std::unordered_set<int> VertexInducedEmbedding::getValidElementsForExpansion() {
	std::unordered_set<int> expansions;
	if (getNumVertices() == 0) {
		expansions.insert(-1); 
	}
	for (int i = 0; i < getNumVertices(); i++) {
		NeighborhoodSet possibleExp = getValidElementsForExpansion(vertices2[i]);
		expansions.insert(possibleExp.begin(), possibleExp.end());
	}

	//remove edges already inserted
	for (int i = 0; i < getNumVertices(); i++) {	
		//std::cout << "removing word: " << vertices[i] << std::endl;
		expansions.erase(vertices2[i]);
	}

	return expansions;
}

std::unordered_map<int,uint> 
VertexInducedEmbedding::getValidElementsForExpansionWithConnectionPattern() {
	std::unordered_map<int,uint> expansions;
	if (getNumVertices() == 0) {
                expansions.insert(std::pair<int,int> (-1, -1) );
        }
        for (int i = 0; i < getNumVertices(); i++) {
                NeighborhoodSet possibleExp = getValidElementsForExpansion(vertices2[i]);
		for (int j : possibleExp) {
			std::unordered_map<int,uint>::iterator it = expansions.find(j);
			if (it==expansions.end()) {
				uint connectionPattern = 0;
				setKthBit(&connectionPattern, i);
				expansions.insert(std::pair<int, int> (j, connectionPattern));
			}
			else {
				setKthBit(&it->second, i);
			}
		}
        }

        //remove edges already inserted
        for (int i = 0; i < getNumVertices(); i++) {
                //std::cout << "removing word: " << vertices[i] << std::endl;
                expansions.erase(vertices2[i]);
        }

        return expansions;
}

std::unordered_set<int> VertexInducedEmbedding::getValidElementsForExpansionWith(std::unordered_set<int> &l) {
	std::unordered_set<int> expansions;
	
	if (getNumVertices() == 0) {
		expansions.insert(-1); 
	}
	for (int i = 0; i < getNumVertices(); i++) {
		NeighborhoodSet possibleExp = getValidElementsForExpansion(vertices2[i]);
		for (int j : possibleExp)
			if (l.find(j)!=l.end()) expansions.insert(j);
	}
	//remove vertices already inserted
	for (int i = 0; i < getNumVertices(); i++) {
		//std::cout << "removing word: " << vertices[i] << std::endl;
		expansions.erase(vertices2[i]);
	}
	return expansions;
}


inline void VertexInducedEmbedding::addWord(int word) {
	if (hasVertex(word)) {
		std::cout << "addWord error! word already exist! " << word << std::endl;
		exit(1);
	}

	//initialize the connection of this vertex
	connections[getNumWords()]=0;
	
	//add edges of node
	int n = 0;
	for (int i = 0; i < getNumVertices(); i++) {
		if (graph->isNeighbor(vertices2[i], word)) {
			setKthBit(&connections[getNumVertices()],i);
			setKthBit(&connections[i],getNumVertices());
			n++;
		}
	}

	//add node
	vertices2[getNumVertices()] = word;
	numVertices++;
}


inline void VertexInducedEmbedding::replaceWord(int word1, int word2) {
	int wordIdx = getVertexPatternId(word1);
	//int n = 0;

	//remove and add edges
	for (int i = 0; i < getNumVertices(); i++) {
		if (i==wordIdx) continue;
		if (graph->isNeighbor(vertices2[i], word2)) {
			setKthBit(&connections[wordIdx],i);
			setKthBit(&connections[i],wordIdx);
			//n++;
		}
		else {
			unsetKthBit(&connections[wordIdx],i);
			unsetKthBit(&connections[i],wordIdx);
		}
	}

	vertices2[wordIdx] = word2;
}


inline void VertexInducedEmbedding::removeLastWord() {
	if (getNumVertices()<=1) {
		reset();
		return;
	}
	
	int word = vertices2[getNumVertices()-1];
	vertices2[getNumVertices()-1] = 0;
	
	//remove edges of node
	for (int i = 0; i < getNumVertices()-1; i++) {
		if (graph->isNeighbor(vertices2[i], word)) {
			unsetKthBit(&connections[i],getNumVertices()-1);
		}
	}
	numVertices--;
}

inline void VertexInducedEmbedding::removeWord(int word) {
	//std::cout << "Remove word " << word << std::endl;
	if (getNumVertices()<=1) {
		reset();
		return;
	}

	if (!hasVertex(word)) {
		std::cout << "problem removeWord! vertex not found!" << std::endl;
		exit(1);
	}

	int wordIdx = getVertexPatternId(word);
	//switch the word with the last one to then remove it
	if (wordIdx!=getNumVertices()-1) {
		vertices2[wordIdx] = vertices2[getNumVertices()-1];
		connections[wordIdx] = connections[getNumVertices()-1];
	}

	//remove from the arrays
	connections[getNumWords()-1] = 0;
	vertices2[getNumVertices()-1] = 0;
	numVertices--;
	
	//fix edges 
	for (int i = 0; i < getNumVertices(); i++) {
		unsetKthBit(&connections[i],getNumVertices());
		if (isKthBitSet(connections[wordIdx],i)) setKthBit(&connections[i],wordIdx);
		else unsetKthBit(&connections[i],wordIdx);
	}
}

int VertexInducedEmbedding::getTotalNumWords() {
	return graph->getNumberOfNodes();
}


int VertexInducedEmbedding::getRandomWordBiased() {
	return graph->getRandomNodeBiased();
}

