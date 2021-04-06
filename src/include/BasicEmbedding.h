#ifndef BASICEMBEDDING_H
#define BASICEMBEDDING_H

#include "constants.h"
#include "utils.h"
#include "graph.h"
#include <utils.hh>
#include <graph.hh>
//#include <bliss_C.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <list>
#include <iostream>
#include <tbb/concurrent_unordered_set.h>


template <typename T>
struct earray: public std::array<T, MAX_EMBEDDING_SIZE>
{
	earray():std::array<T, MAX_EMBEDDING_SIZE>(){};
	friend std::ostream& operator<<(std::ostream&os, const earray<T> &a){
		
		os << "[";
		for (T i : a) os << " " << i;
		os << " ]";
		return os;
	}
};

template <typename T>
    struct earrayhash
    {
        size_t operator()(const earray<T> &a) const {
		return hash(a);
	}

        static size_t hash(const earray<T> &a) 
	{   
	    std::hash<uint> hasher;
            size_t h = 0;
            for (size_t i = 0; i < MAX_EMBEDDING_SIZE; ++i)
                h = h * 31 + hasher(a[i]);
            return h;
        }

        static bool equal(const earray<T>& a, const earray<T> &b) 
	{
		return a==b;
	}	
    };



struct Mod {
	public:
		int rmId;
		int addId;
		double totalWeight;

		Mod():rmId(-1), addId(-1), totalWeight(0) {};
		Mod(int r, int a, double w): rmId(r), addId(a), totalWeight(0) {}
		Mod(int r, int a): rmId(r), addId(a), totalWeight(0) {}

		Mod inverse() { return Mod(addId, rmId); }
		void print() const { std::cout << "Mod: " << rmId << " " << addId << " " << totalWeight << std::endl; }
		friend std::ostream& operator<<(std::ostream& os, const Mod& mod) {
			os.setf(std::ios::fixed);
			os << "Mod: { rmId: " <<  mod.rmId << " addId: " << mod.addId << "totalWeight: " << mod.totalWeight<< " }"; 
			return os;
		}
		bool operator==(const Mod &other) const {
		   if(this->addId==other.addId && this->rmId==other.rmId) return true;
		   else return false;
		}	
};

struct mod_hash {
    std::size_t operator()(Mod const &mod) const {
        size_t seed = 0;
        boost::hash_combine(seed, mod.addId * 2654435761);
        boost::hash_combine(seed, mod.rmId * 2654435761);
        return seed;
    }
};

typedef std::unordered_set<Mod, mod_hash> ModSet;

class EmbKey : public std::vector<int> {
	EmbKey():vector<int>() {};
};

class BasicEmbedding {

protected:
   // Basic structure 
   Graph *graph;
   //int numNeighbors;
   double degree;
   int numVertices;
   
   earray<int> vertices2;
   earray<uint> connections;
	
public : 
     bool isPoisonPill;

     BasicEmbedding();
     BasicEmbedding(Graph *g);
     BasicEmbedding(Graph *g, earray<int>&, int);
     virtual ~BasicEmbedding();
     
     void reset();
     bool isEmpty();
     void init();
     
     Graph *getGraph();
     earray<int> &getVertices();
     int getNumVertices();
     earray<uint> &getConnections();
     bool hasVertex(int);
     bool hasEdge(int);
     int getVertexPatternId(int);
     
     virtual void removeWord(int wordId) = 0;
     virtual void removeLastWord() = 0;
     virtual void addWord(int) = 0;
     virtual void replaceWord(int, int) = 0;
     virtual earray<int> &getWords() = 0; 
     virtual bool hasWord(int) = 0; 
    
     virtual std::vector<int> getValidElementsForExpansionSorted() = 0;
     virtual std::unordered_map<int, uint> getValidElementsForExpansionWithConnectionPattern() = 0;
     virtual std::vector<int> &getValidElementsForExpansionSorted(int) = 0;
     virtual std::unordered_set<int> getValidElementsForExpansion() = 0;
     virtual NeighborhoodSet &getValidElementsForExpansion(int) = 0;
     virtual std::unordered_set<int> getValidElementsForExpansionWith(std::unordered_set<int>&) = 0;
     virtual std::unordered_set<int> getValidElementsForContraction() = 0;
     virtual std::unordered_set<int> getValidElementsForContractionWithWord(int) = 0;
     virtual NeighborhoodSet getWordNeighbors(int) = 0;
     
     NeighborhoodSet getWordNeighbors();
     int getNumberOfWordNeighbors(int);
     uint getWordDegree(int);
     virtual int getTotalNumWords() = 0;
     virtual int getNumWords() = 0;
     earray<int> getWordsSorted(); 
     
     std::pair<size_t,int> getWordConnectionHash(int);	

     virtual int getRandomWordBiased() = 0;
     int findWordPosition(int word); 
     int getNodeDegree();
     bool isCanonicalEmbeddingWithWord(int);
     std::string toString();
     int getNumberOfSharedWordIds(BasicEmbedding &);
     //int getNumberOfNonSharedWordIds(BasicEmbedding &);
     std::set<int> getSharedWordIds(BasicEmbedding &);
     //std::vector<int> getNonSharedWordIds(BasicEmbedding &);
     //std::vector<int> getDiffWordIds(BasicEmbedding &);
     //bool isSmaller(BasicEmbedding&);
     bool isSameEmbedding(BasicEmbedding&);
     bool isSamePattern(BasicEmbedding&);
    
     bool isConnected();
     double getDegree() const;
     void setDegree(double );
     void setGraph(Graph *);
     virtual bool areWordsNeighbours(int wordId1, int wordId2) = 0;
     virtual void print();
     void loadFromString(std::string &);
     void writeWordsToFile(std::ofstream &);
	
     friend std::ostream& operator<<(std::ostream&os , BasicEmbedding& e){
	os.setf(std::ios::fixed);
       	earray<int> vertices = e.getVertices();
       	//std::vector<int> edges = e.getEdges();
	//
	std::sort(vertices.begin(), vertices.begin()+e.getNumVertices());
	os << "Embedding: { ";
	os << "vertices: [ ";
	for (int i = 0; i< e.getNumVertices(); i++) {
		os << vertices[i] << " ";
	}
	os << "]";
	//os << " edges: [ ";
	//for (int i = 0; i< (int)edges.size(); i++) {
	//	os << edges[i] << " ";
	//}
	os << "]";
	os << " }";
	

	return os;	
   };

   double quasiCliqueScore() {
	earray<int> &words = this->getWords();
	double min_factor = 1.;
	
	for (int u = 0; u < this->getNumWords(); ++u) {
		double factor = (double) this->getWordDegree(words[u])/(this->getNumWords() - 1.);
		if (min_factor>factor) min_factor = factor;
	}


	/*earray<uint> degreeDist;
	uint k = this->getNumWords();
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
	else if (min_factor != 2./3) {
		std::cout << "other! " << min_factor << std::endl;
		exit(1);
	}*/
	

	return min_factor;
   }

};
     
class CompareEmbeddings{
    public:
    bool operator() (const BasicEmbedding &l, const BasicEmbedding &r) const {
        return (l.getDegree() > r.getDegree());
    }
};



#endif
