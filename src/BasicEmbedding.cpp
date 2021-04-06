#include "BasicEmbedding.h"
#include "EmbeddingUtils.h"
#include "canonical.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>

    //BasicEmbedding::BasicEmbedding():neighborhoodSet() {
BasicEmbedding::BasicEmbedding() {
	init();
	isPoisonPill = false;
	degree=0;
	numVertices=0;
}

BasicEmbedding::BasicEmbedding(Graph *g):BasicEmbedding() {
	graph = g;
}

BasicEmbedding::BasicEmbedding(Graph *g, earray<int> &words, int numWords):BasicEmbedding(g) {
}

BasicEmbedding::~BasicEmbedding() {
};

void BasicEmbedding::init() {
	reset();
}

void BasicEmbedding::reset() { 
	vertices2.fill(0);
	connections.fill(0);
	degree=0;
	numVertices=0;
}

bool BasicEmbedding::isEmpty() {
	return this->getNumWords()==0;
}

double BasicEmbedding::getDegree() const {
	return degree;
}

uint BasicEmbedding::getWordDegree(int i) {
	return bitCount(connections[i]);
}

earray<int> &BasicEmbedding::getVertices() {
	return vertices2;
}

int BasicEmbedding::getNumVertices() {
	return numVertices;
}

earray<uint> &BasicEmbedding::getConnections() {
	return connections;
}


Graph *BasicEmbedding::getGraph() {
	return graph;
}

bool BasicEmbedding::hasVertex(int v) {
	return std::find(vertices2.begin(), vertices2.begin()+getNumVertices(), v) != vertices2.begin()+getNumVertices();
}

earray<int> BasicEmbedding::getWordsSorted() {
	earray<int> words(getWords());
	std::sort(words.begin(), words.begin()+getNumWords());
	return words;
}

void BasicEmbedding::setDegree(double d) {
	degree = d;
}

void BasicEmbedding::setGraph(Graph *g) {
	graph = g;
}

bool BasicEmbedding::isCanonicalEmbeddingWithWord(int wordId) {
	earray<int> &words = getWords();
	int numWords = getNumWords();

	if (numWords == 0) return true;
	if (wordId < words[0]) return false;

	int i;

	// find the first neighbor
	for (i = 0; i < numWords; ++i) {
		if (areWordsNeighbours(wordId, words[i])) {
			break;
		}
	}

	// if we didn't find any neighbour
	if (i == numWords) {
		// not canonical because it's disconnected
		std::cout << "try to add a invalid vertex!! canonicality checking fail!" << std::endl;
		exit(1);
	}

	// If we found the first neighbour, all following words should have lower
	// ids than the one we are trying to add
	i++;
	for (; i < numWords; ++i) {
		// If one of those ids is higher or equal, not canonical
		if (words[i] >= wordId) {
			return false;
		}
	}

	return true;
}

std::string BasicEmbedding::toString() {
	/*return "Embedding{" +
	  "vertices=" + vertices + ", " +
	  "edges=" + edges +
	  "} " + super.toString();
	  */
	return std::string("bla");
}

bool BasicEmbedding::isSameEmbedding(BasicEmbedding &e) {
	if (this->getNumberOfSharedWordIds(e) == this->getNumWords()) { 
		return true;
	}
	return false;
}

bool BasicEmbedding::isSamePattern(BasicEmbedding &e) {
	if (Canonical::getHash2(e)!=Canonical::getHash2(*this)) 
		return false;
	return true;
}

int BasicEmbedding::getVertexPatternId(int v) {
	earray<int>::iterator it = std::find(vertices2.begin(), vertices2.begin()+getNumVertices(), v);
	if (it == vertices2.begin()+getNumVertices()) return -1;
	return std::distance(vertices2.begin(),it);
}

int BasicEmbedding::getNumberOfSharedWordIds(BasicEmbedding &embedding) {
	std::set<int> shared =  this->getSharedWordIds(embedding);
	return shared.size();
}

std::set<int> BasicEmbedding::getSharedWordIds(BasicEmbedding &embedding) {
	std::set<int> shared;
	earray<int> &words = getWords();
	shared.insert(words.begin(), words.begin()+getNumWords());
	
	earray<int> &otherWords = embedding.getWords();
	shared.insert(otherWords.begin(), otherWords.begin()+embedding.getNumWords());
	/*earray<int> sortedWords(this->getWords());
	earray<int> sortedOtherWords(embedding.getWords());

	std::sort(sortedWords.begin(), sortedWords.begin()+getNumWords());
	std::sort(sortedOtherWords.begin(), sortedOtherWords.begin()+embedding.getNumWords());

	int i=0, j=0;
	while (i < getNumWords() && j < embedding.getNumWords()) {
		if (sortedWords[i] == sortedOtherWords[j]) {
			shared.insert(sortedWords[i]);
			i++;
			j++;
		}
		else if (sortedWords[i] < sortedOtherWords[j])
			i++;
		else
			j++;
	}*/
	return shared;
}

/*std::vector<int> BasicEmbedding::getDiffWordIds(BasicEmbedding &other) {
	std::vector<int> diff;
	std::vector<int> sortedWords(this->getWords());
	std::vector<int> sortedOtherWords(other.getWords());
	std::sort(sortedWords.begin(), sortedWords.end());
	std::sort(sortedOtherWords.begin(), sortedOtherWords.end());

	int i=0, j=0;
	while (i < (int)sortedWords.size() && j < (int)sortedOtherWords.size()) {
		if (sortedWords[i] == sortedOtherWords[j]) {
			i++;
			j++;
		}
		else if (sortedWords[i] < sortedOtherWords[j]) {
			diff.push_back(sortedWords[i]);
			i++;
		}
		else {
			j++;
		}
	}
	while (i < (int)sortedWords.size()) {
		diff.push_back(sortedWords[i]);
		i++;
	}

	return diff;
}*/


bool BasicEmbedding::isConnected() {
	return ((int)bitCount(EmbeddingUtils::dfs(*this)) == getNumVertices());
}

std::pair<size_t,int> BasicEmbedding::getWordConnectionHash(int wordId) {
	earray<int> words = getWords();
	size_t seed = 0;
	int n = 0;
	for (int i = 0; i < getNumWords(); ++i) {
		if (wordId != words[i] && areWordsNeighbours(wordId, words[i])) {
			boost::hash_combine(seed,  words[i] * 2654435761);
			n++;
		}
	}
	return std::pair<size_t, int> (seed,n);
}

void BasicEmbedding::print() {
	earray<int> &vertices2 = getVertices();
	std::cout << "Embedding: { ";
	std::cout << "vertices: [ ";
	for (int i = 0; i < getNumVertices(); i++) {
		std::cout << vertices2[i] << " ";
	}
	std::cout << "]";
	//std::cout << " edges: [ ";
	//for (int i = 0; i< (int)edges.size(); i++) {
	//	std::cout << edges[i] << " ";
	//}
	//std::cout << "]";
	std::cout << " }";
	//std::cout << " HASH: " << this->getHash();
	std::cout << std::endl;

	} 

	void BasicEmbedding::loadFromString(std::string &s) {
		std::vector<std::string> tokenList;
		boost::split(tokenList,s, boost::is_any_of(" ,;"));
		for (auto& i : tokenList) {
			addWord(atoi(i.c_str()));
		}
	}

	void BasicEmbedding::writeWordsToFile(std::ofstream & os) {
		assert(os.is_open());

		os << vertices2[0];
		for (int i = 1; i< getNumVertices(); i++)
			os << "," << vertices2[i];
		os << std::endl;
	}

	int BasicEmbedding::getNumberOfWordNeighbors(int id) {
		return (getWordNeighbors(id)).size();
	}

	NeighborhoodSet BasicEmbedding::getWordNeighbors() {
		NeighborhoodSet neighs;
		earray<int> words = getWords();
		for (int i = 0; getNumWords(); i++) {
			NeighborhoodSet localNeigh = getWordNeighbors(words[i]);
			neighs.insert(localNeigh.begin(), localNeigh.end());
		}
		return neighs;
	}

