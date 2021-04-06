#ifndef TOURSTATS_H
#define TOURSTATS_H

#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

#include "random.h"
#include "reservoir.h"

template <class T>
class TourStats2 {
   public:
    int _p;
    int _s;
    uint128_t steps;
    uint128_t stepsRW;
    float128 stepsSecMoment;
    uint numTours;
    bool ret;
    float128 n;
    float128 n2;
    //std::unordered_map<earray<uint>*, float128, earrayhash<uint>> ns;
    std::unordered_map<earray<uint>*, float128> ns;
    //std::vector<reservoir<T>> rs;

    TourStats2() {
		_p = 0;
        	_s = 0;
		steps = 0;
		stepsRW = 0;
		stepsSecMoment = 0;
		numTours = 0;
		ret = true;
		n = 0;
		n2 = 0;
    };

    TourStats2(int p, int s) {
        _p = p;
        _s = s;
        steps = 0;
        stepsRW = 0;
	stepsSecMoment = 0;
	numTours = 0;
        ret = true;
        n = 0;
        n2 = 0;
        //rs.resize(p, reservoir<T>(s));
    };

    void reset() {
        steps = 0;
        stepsRW = 0;
	stepsSecMoment = 0;
	numTours = 0;
        ret = true;
        n = 0;
        n2 = 0;
        ns.clear();
    }

    void insert(earray<uint> *hash, double d) {
        std::unordered_map<earray<uint>*, float128>::iterator it = ns.find(hash);
        //std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator it = ns.find(*hash);
        if (it == ns.end())
            //ns.insert(std::pair<earray<uint>, float128>(*hash, d));
            ns.insert(std::pair<earray<uint>*, float128>(hash, d));
        else
            it->second += d;
    }

    TourStats2& operator+=(const TourStats2& tourStats) {
        if (!tourStats.ret || !this->ret) {
            std::cout << "EXIT: Tour stats is incomplete. Some tour did not completed. " << ret << " " << steps << " " << tourStats.ret << " " << tourStats.steps << std::endl;
            std::_Exit(EXIT_FAILURE);
        }

        steps += tourStats.steps;
        stepsRW += tourStats.stepsRW;
	stepsSecMoment += tourStats.stepsSecMoment;
 	numTours += tourStats.numTours;
        n += tourStats.n;
        n2 += tourStats.n2;

        for (std::pair<earray<uint>*, float128> p : tourStats.ns) {
        //for (std::pair<earray<uint>, float128> p : tourStats.ns) {
            //std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator it = this->ns.find(p.first);
            std::unordered_map<earray<uint>*, float128>::iterator it = this->ns.find(p.first);
            if (it == this->ns.end())
                this->ns.insert(p);
            else
                it->second += p.second;
        }

        return *this;
    }

    TourStats2 operator+(TourStats2& tourStats) {
        if (!tourStats.ret || !this->ret) {
            std::cout << "EXIT: Tour stats is incomplete. Some tour did not completed. " << this->ret << " " << this->steps << " " << tourStats.ret << " " << tourStats.steps << std::endl;
            std::_Exit(EXIT_FAILURE);
        }

        TourStats2 aggStats(tourStats._p, tourStats._s);
        aggStats.steps = tourStats.steps + this->steps;
        aggStats.stepsRW = tourStats.stepsRW + this->stepsRW;
	aggStats.stepsSecMoment = tourStats.stepsSecMoment + this->stepsSecMoment;
        aggStats.numTours = tourStats.numTours + this->numTours;
        aggStats.n = tourStats.n + this->n;
        aggStats.n2 = tourStats.n2 + this->n2;

        aggStats.ns.insert(this->ns.begin(), this->ns.end());
        for (std::pair<earray<uint>*, float128> p : tourStats.ns) {
        //for (std::pair<earray<uint>, float128> p : tourStats.ns) {
            //std::unordered_map<earray<uint>, float128, earrayhash<uint>>::iterator it = aggStats.ns.find(p.first);
            std::unordered_map<earray<uint>*, float128>::iterator it = aggStats.ns.find(p.first);
            if (it == aggStats.ns.end())
                aggStats.ns.insert(p);
            else
                it->second += p.second;
        }
        return aggStats;
    }

    void print() {
        std::cout.setf(std::ios::fixed);
        std::cout << "TourStats: { steps: " << steps;
        std::cout << " stepsRW: " << stepsRW;	
        std::cout << " stepsSecMoment: " << stepsSecMoment;	
        std::cout << " numTours: " << numTours;	
        std::cout << " n: " << n;
        std::cout << " n2: " << n2;
        std::cout << "}";
    }

    friend std::ostream& operator<<(std::ostream& os, TourStats2& stats) {
        os.setf(std::ios::fixed);
        os.precision(std::numeric_limits<double>::max_digits10);
        os << "TourStats: { steps: " << stats.steps;
        os <<  " stepsRW: " << stats.stepsRW;	
        os << " stepsSecMoment: " << stats.stepsSecMoment;	
        os <<  " numTours: " << stats.numTours;	
        os << " n: " << stats.n;
        os << " n2: " << stats.n2;
        os << "}";
        return os;
    }
};

typedef TourStats2<earray<int>> TourStatsWords;

#endif
