#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <limits.h>
#include <list>
#include <vector>
#include <set>
#include <fstream>      // std::ifstream
#include <iostream>
#include <queue>          // std::priority_queue
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>    // std::find
#include <sys/resource.h>
#include <mutex>


typedef boost::multiprecision::cpp_bin_float_quad float128;
typedef boost::multiprecision::uint128_t uint128_t;
typedef unsigned int Uint;
typedef unsigned int uint;
typedef short unsigned int SUint;
typedef long unsigned int LUint;
typedef unsigned int COUNT;
typedef unsigned int LABEL;
const int MAXUINT = UINT_MAX;
const double REDUCTION_FACTOR = 100000.;

struct mutex_wrapper : std::mutex
{
  mutex_wrapper() = default;
  mutex_wrapper(mutex_wrapper const&) noexcept : std::mutex() {}
  bool operator==(mutex_wrapper const&other) noexcept { return this==&other; }
};


struct VectorCompWithOthers
{
    const std::vector<uint> &value_vector;
    const std::vector<uint> &value_vector2;
    VectorCompWithOthers(const std::vector<uint> &val_vec, const std::vector<uint> &val_vec2):value_vector(val_vec), value_vector2(val_vec2) {}

    bool operator()(int i1, int i2)
    {
        return value_vector[i1] != value_vector[i2] ? value_vector[i1] < value_vector[i2] : value_vector2[i1] < value_vector2[i2];
    }

    bool equal(int i1, int i2) {
	    return value_vector[i1] == value_vector[i2] && value_vector2[i1] == value_vector2[i2];
    }

};

struct VectorCompWithOther
{
    const std::vector<uint> &value_vector;
    VectorCompWithOther(const std::vector<uint> &val_vec):value_vector(val_vec) {}

    bool operator()(int i1, int i2)
    {
        return value_vector[i1] < value_vector[i2];
    }
    bool equal(int i1, int i2) {
	    return value_vector[i1] == value_vector[i2];
    }
};



// Only for pairs of std::hash-able types for simplicity.
// You can of course template this struct to allow other hash functions
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;
    }
};

typedef std::unordered_map<std::string, int> StrIntMap;
typedef std::unordered_map<int,std::string> IntStrMap;
typedef std::unordered_map<int,int> IntIntMap;
typedef std::unordered_map<std::string, std::string> StrStrMap;

class Config {
	public:
	static StrStrMap configMap;

    static void load(std::string);
    static void load(std::string,std::string);
	static void reload( std::string);
	static bool isActive();
	static uint getKeyAsUint(std::string);
	static int getKeyAsInt(std::string);
	static std::pair<int,int> getKeyAsIntPair(std::string);
	static double getKeyAsDouble(std::string);
	static std::string getKeyAsString(std::string);
	static bool getKeyAsBoolean(std::string);
	static bool existKey(std::string);
	static void print();
};

template <typename Container> // we can make this generic for any container [1]
struct container_hash {
    std::size_t operator()(Container const& c) const {
    //std::size_t operator()(std::vector<int> const& c) const {
	size_t seed = 0;
	for (const int& i : c)
		boost::hash_combine(seed,  i * 2654435761);
        //return boost::hash_range(c.begin(), c.end());
	return seed;

    }
};


typedef std::unordered_map<std::vector<int>, int, container_hash<std::vector<int> >> VectorIntIntMap;
typedef std::unordered_map<std::vector<int>, int, container_hash<std::vector<int> >>::iterator VectorIntIntMapIt;
typedef std::unordered_map<std::set<int>, int, container_hash<std::set<int> >> SetIntIntMap;
typedef std::unordered_map<std::set<int>, int, container_hash<std::set<int> >>::iterator SetIntIntMapIt;


void mapIdToLabels(const StrIntMap&, IntStrMap&);
bool isPrime(int);
void printIntList(std::list<int> &);
bool compare_pair(const std::pair<int, double> &, const std::pair<int, double> &);
void setStackSize();

template<typename T, typename C>
class TopQueue:public std::priority_queue<T, std::vector<T>, C> {
        private :
        int maxSize;
 
        public :
        TopQueue<T,C> (int s):std::priority_queue<T, std::vector<T>, C >(){maxSize=s;};
        TopQueue<T,C> ():std::priority_queue<T, std::vector<T>, C>(){};
        
	void setMaxSize (int s) { maxSize = s;};
        int getMaxSize () { return maxSize;};
 
        std::pair<T, bool> getTop () {
              if (this->size() == maxSize) return std::pair<T, bool> (this->top(), true);
              return std::pair<T, bool> (this->top(), false);
        }
 
        void insert(T t) {
            this->push(t);
            if ((int)this->size() > maxSize) this->pop();
        }
        
	void insertVectorOnly(T t) {
	    this->c.push_back(t);
        }

	//TODO FIX IT
	bool remove(const T& value) {
		/*if (it != this->c.end()) {
        	    this->c.erase(it);
        	    std::make_heap(this->c.begin(), this->c.end(), this->comp);
        	    return true;
       		}	
       		else {
       	 		return false;
       		}*/
		return false; 

	}

	void print() {
		std::vector<T> q = this->c;
		for (int i = 0; i < q.size(); i++) {
			q[i].print();
		}
	}
	
	void clear() {
		this->c.clear();
        	std::make_heap(this->c.begin(), this->c.end(), this->comp);
	}
	
	void remakeHeap() {
        	std::make_heap(this->c.begin(), this->c.end(), this->comp);
	}

	std::vector<T> &getVector() {
		return this->c;
	}
	
	std::vector<T> getVectorConst() const {
		return this->c;
	}

	void setVector(std::vector<T> &c) {
		this->c = c;
        	std::make_heap(this->c.begin(), this->c.end(), this->comp);
	}

	void reserve(int r) {
		this->c.reserve(r);
	}
};

class CompareIntDec{
    public:
    bool operator() (const int l, const int r) const {
        return (l > r);
    }
};

class CompareIntInc{
    public:
    bool operator() (const int l, const int r) const {
        return (l > r);
    }
};

class CompareIntIntPairDec{
    public:
    bool operator() (const std::pair<int, int> &l, const std::pair<int,int> &r) const {
        return (l.second < r.second);
    }
};

class CompareIntIntPairInc{
    public:
    bool operator() (const std::pair<int, int> &l, const std::pair<int,int> &r) const {
        return (l.second > r.second);
    }
};

class CompareIntDoublePairInc{
    public:
    bool operator() (const std::pair<int, double> &l, const std::pair<int, double> &r) const {
        return (l.second > r.second);
    }
}; 

class CompareIntDoublePairDec{
    public:
    bool operator() (const std::pair<int, double> &l, const std::pair<int, double> &r) const {
        return (l.second < r.second);
    }
};

inline uint bitCount(uint i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    i = (i + (i >> 4)) & 0x0f0f0f0f;
    i = i + (i >> 8);
    i = i + (i >> 16);
    return i & 0x3f;
}

inline void setKthBit(uint *n, int k) 
{ 
    *n |= (1 << k); 
} 

inline void unsetKthBit(uint *n, int k) 
{ 
    *n &= ~(1 << k); 
} 

inline uint minusBitSet(uint n, int k) 
{   
    return (n|k) & ~k;
} 

inline uint unionBitSet(uint n, int k) 
{   
    return n|k;
} 

inline bool isKthBitSet(uint n, int k) 
{ 
    if (n & (1 << (k))) 
	return true;
    else
	return false;
} 

#endif

