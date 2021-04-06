#ifndef RESERVOIR_HPP
#define RESERVOIR_HPP

#include <vector>
#include <tbb/concurrent_vector.h>
#include <math.h>
#include "random.h"
#include "utils.h"


// a cache which evicts the least recently used item when it is full
template<typename O>
class reservoir {

	protected:
	const static uint DEFAULT_CAPACITY = 10;
	
	public:
	size_t id;
	uint cpty;
	tbb::atomic<long unsigned int> total;
	tbb::concurrent_vector<O> rvr;
	mutex_wrapper my_mtx;

	reservoir(uint c): id(0), cpty(c), total(0) { 
	};
	
	reservoir(size_t i, uint c): id(i), cpty(c), total(0) {

	};	

	reservoir(): id(0), cpty(DEFAULT_CAPACITY), total(0) { 
	};

	~reservoir() { 
	};


	bool empty() const { return total==0; };

        inline bool insert(O obj) {
            uint idx = ++total;

            if (idx == std::numeric_limits<uint>::max()) {
                std::cout << "error: reservoir total is large than max allowed" << std::endl;
            }

            if (idx - 1 < cpty) {
                uint current_size = rvr.size();
                uint new_size = std::max<uint>(current_size * 2, 10000);
                new_size = std::min<uint>(new_size, cpty);

                if (idx - 1 >= current_size) {
                    my_mtx.lock();
                    if (idx - 1 >= rvr.size()) {
                        //rvr.resize(rvr.size()+100000, obj);
                        try {
                            rvr.resize(new_size, obj);
                        } catch (std::bad_alloc const& e) {
                            std::cout << e.what() << std::endl;
                            std::exit(134);
                        }
                        std::cout << "increasing reservoir " << id << " idx " << idx - 1 << " " << rvr.size() << std::endl;
                    }
                    my_mtx.unlock();
                }
                rvr[idx - 1] = obj;
                //rvr.push_back(obj);
                return true;
            } else {
                double prob = Randness::getInstance()->random_uni01();
                if (log(prob) <= (log(cpty) - log(idx))) {
                    rvr[Randness::getInstance()->get_a_random_number(0, cpty)] = obj;
                    return true;
                }
                return false;
            }

            /*if (total < cpty) {
			//rvr[total++] = obj; 	
			total++;
			rvr.push_back(obj);
			return true;
                }
		else {	
			double prob = Randness::getInstance()->random_uni01();
			if (prob <= (double) 1./(++total)) {
				rvr[Randness::getInstance()->get_a_random_number(0, cpty)] = obj;
				return true;
			}
			return false;
		}*/
        }

        void reset() {
		total = 0;
		rvr.clear();
	}

	O &random() {
		/*if (total==0) {
			std::cout << "error: impossible to get a random element in reservoir" << std::endl;
			exit(1);
		}*/
                uint high = cpty; 
                if (total < cpty) high = (uint) total;

		uint j = Randness::getInstance()->get_a_random_number(0, high);
	
		O nullObj;
                /*if (j >= cpty || rvr[j] == nullObj) {
                        std::cout << "error: reservoir " << id << " random element position: " << j << " " << 0 << " "<< high << std::endl;
			exit(1);
		}*/
		return rvr[j];
	}
	
	/*O &randomWithBias() {
		if (filled <=0) {
			std::cout << "error: impossible to get a random element in reservoir" << std::endl;
			exit(1);
		}
		
		double totalWeight = 0;
		int j = -1;
		for(uint i = 0; i < filled; ++i) {
			double prob = Randness::getInstance()->random_uni01();
			if (prob <= (double) weights[i]/(totalWeight+weights[i])) {
				j = i;		
			}
			totalWeight+=weights[i];
		}
		
		//std::cout << "getting random element in reservoir position: " << j << std::endl;
		return rvr[j];
	}*/


	void copy(reservoir<O> r) {
		cpty = r.cpty;
		total = r.total;
		rvr = r.rvr;
	}

	reservoir<O> operator+ (const reservoir<O> &r2) {
		if (this->cpty != r2.cpty) {
			std::cout << "error: can not merge the reservoirs!" << std::endl;
			exit(1);
		}
		
		return this;
	}

	reservoir<O>& operator+= (const reservoir<O> &r2) {
		if (this->cpty != r2.cpty) {
			std::cout << "error: can not merge the reservoirs!" << std::endl;
			exit(1);
		}
	
		if (r2.filled==0) return *this;
 	
		return *this;
	}

	void print () {
		std::cout << "reservoir: " << " total=" << total << " filled=" << rvr.size() << " capacity="<< cpty << std::endl;
		for(uint i = 0; i < rvr.size(); ++i) {
			std::cout << rvr[i] << std::endl;	
		}	
	}
	
	friend std::ostream& operator<<(std::ostream& os, reservoir<O>& r) {
		os << "reservoir: " << " total=" << r.total << " filled=" << r.rvr.size() << " capacity="<< r.cpty << std::endl;
		for(uint i = 0; i < r.rvr.size(); ++i) {
			std::cout << r.rvr[i] << std::endl;	
		}	
		return os;	
	}
};


#endif 
