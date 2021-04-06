//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#ifndef BOOST_COMPUTE_DETAIL_LRU_CACHE_HPP
#define BOOST_COMPUTE_DETAIL_LRU_CACHE_HPP

#include <map>
#include <list>
#include <utility>
#include <iostream>

#include <boost/optional.hpp>
//#include <tbb/concurrent_unordered_map.h>

// a cache which evicts the least recently used item when it is full
template<class Key, class Value>
class lru_cache
{
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef std::list<key_type> list_type;
    typedef std::map<
                key_type,
                std::pair<value_type, typename list_type::iterator>
            > map_type;

    lru_cache(size_t capacity)
    {
	hit = 0;
	miss = 0;
	m_capacity = capacity;
    }
    
    lru_cache()
    {
	hit = 0;
	miss = 0;
	m_capacity = DEFAULT_CAPACITY;
    }

    ~lru_cache()
    {
    }

    size_t size() const
    {
        return m_map.size();
    }

    size_t capacity() const
    {
        return m_capacity;
    }

    void setCapacity(size_t capacity) 
    {
        m_capacity = capacity;
    }

    bool empty() const
    {
        return m_map.empty();
    }

    bool contains(const key_type &key)
    {
        return m_map.find(key) != m_map.end();
    }

    inline void insert(const key_type &key, const value_type &value)
    {
        typename map_type::iterator i = m_map.find(key);
        if(i == m_map.end()){
            // insert item into the cache, but first check if it is full
            if(size() >= m_capacity){
                // cache is full, evict the least recently used item
                evict();
            }

            // insert the new item
            m_list.push_front(key);
            m_map[key] = std::make_pair(value, m_list.begin());
        }
    }
    
    inline boost::optional<value_type&> get(const key_type &key)
    {
        // lookup value in the cache
        typename map_type::iterator i = m_map.find(key);
        if(i == m_map.end()){
            // value not in cache
	    miss++;
            return boost::none;
        }

	hit++;
        // return the value, but first update its place in the most
        // recently used list
        typename list_type::iterator j = i->second.second;
        if(j != m_list.begin()){
            // move item to the front of the most recently used list
            m_list.erase(j);
            m_list.push_front(key);

            // update iterator in map
            j = m_list.begin();
            //value_type &value = i->second.first;
            //m_map[key] = std::make_pair(value, j);

	    i->second.second = j; 
        }
        
	return i->second.first;
    }

    void updateList(const key_type &key) {
        typename map_type::iterator i = m_map.find(key);
        if(i == m_map.end()) return;
	
	//update its place in the most
        // recently used list
        typename list_type::iterator j = i->second.second;
        if(j != m_list.begin()){
            // move item to the front of the most recently used list
            m_list.erase(j);
            m_list.push_front(key);

            // update iterator in map
            j = m_list.begin();
            //value_type &value = i->second.first;
            //m_map[key] = std::make_pair(value, j);

            i->second.second = j;
        }
    }

    void resetCounters() {
	hit = 0;
	miss = 0;
    }

    void clear()
    {
        m_map.clear();
        m_list.clear();
	hit = 0;
	miss = 0;
    }

    size_t getHit()
    {
	return hit;
    }

    size_t getMiss() 
    {
	return miss;
    }
    
    void setHit(size_t h)
    {
	hit = h;
    }

    void setMiss(size_t m) 
    {
	miss = m;
    }

    size_t getAccess()
    {
	return miss+hit;
    }

    void printResume() {
	double ratio = 0;
	if (miss+hit!=0) ratio = (double) hit/(miss+hit);
	std::cout << "CACHE ANALYSIS: {" << " capacity: " << m_capacity << " total: "<< miss+hit << " hits: " << hit << " = " << ratio << "% }" << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, lru_cache<Key,Value> &cache) {
	double ratio = 0;
	if (cache.miss+cache.hit!=0) ratio = (double) cache.hit/(cache.miss+cache.hit);
	os << "CACHE ANALYSIS: {" << " capacity: " << cache.m_capacity << " total: "<< cache.miss+cache.hit << " hits: " << cache.hit << " = " << ratio << "% }" << std::endl;
	return os;
    }

protected:
    void evict()
    {
        // evict item from the end of most recently used list
        typename list_type::iterator i = --m_list.end();
        m_map.erase(*i);
        m_list.erase(i);
    }

protected:
    const int DEFAULT_CAPACITY = 10000;
    map_type m_map;
    list_type m_list;
    size_t m_capacity;

    size_t miss; 
    size_t hit; 
};



#endif // BOOST_COMPUTE_DETAIL_LRU_CACHE_HPP
