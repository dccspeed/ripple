#ifndef UTILSRANDOM_H
#define UTILSRANDOM_H

#include <ctime>
#include <cmath>
#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <thread>
#include <tbb/task_arena.h>
#include "constants.h"
#include "SFMT.h"
#include "pcg_random.hpp"

class Randness {

private:

static Randness *instance;
 
Randness();

public:

//~Randness();

static Randness *getInstance() {
    if (instance==NULL) instance = new Randness();
    return instance;
}

inline double random_uni01() {
	double r = 0.;
        thread_local std::mt19937 generator(std::random_device{}());
     	boost::random::uniform_real_distribution<double> distribution(0.0,1.0);
	r=distribution(generator);
	return r;
}

inline unsigned int get_a_random_number(int lowest, int highest) {
	double r = 0.;
	//std::cout << "lowest: " << lowest << " highest: " << highest << std::endl;
	if (highest-1 == lowest) {
		return lowest;		
  	} 
  	else if (highest-1 < lowest) {
	    	std::cout << "ERROR In random_number_generator: Higher value is smaller than lower" << std::endl;
	    	exit(1);
  	}
        thread_local std::mt19937 generator(std::random_device{}());
	boost::random::uniform_int_distribution<int> distribution(lowest,highest-1);
	r=distribution(generator);
	return r;
}

//get random element from container
template <typename I>
inline I random_element(const I begin, const I end)
{
    const unsigned long n = std::distance(begin, end);

    I r = begin;
    std::advance(r, get_a_random_number(0,n));
    
    return r;
}

/*! \fn unsigned int randomWithDiscreteProbability(const vector<double>& accum_prob_vec) 
 		*  \brief A function to return a random number with discrete probability; pass the cum. distribution vector.
 		*  \param accum_prob_vec a constant reference of double vector.
		*	 \return an unsigned integer
 		*/
inline unsigned int randomWithDiscreteProbability(const std::vector<double>& accum_prob_vec) {
  double x = random_uni01();
  return lower_bound(accum_prob_vec.begin(), accum_prob_vec.end(), x) - 
         accum_prob_vec.begin();
}

inline unsigned int randomWithDiscreteProbability(const std::vector<int>& accum_prob_vec) {
  int highest = accum_prob_vec.back()+1;
  int x = get_a_random_number(0, highest);
  return lower_bound(accum_prob_vec.begin(), accum_prob_vec.end(), x) - 
         accum_prob_vec.begin();
}

};

#endif
