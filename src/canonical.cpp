#include "canonical.h"

tbb::atomic<int> Canonical::hits = 0;
tbb::atomic<int> Canonical::misses = 0;
tbb::concurrent_hash_map<size_t, size_t> Canonical::pattern_to_canonical = tbb::concurrent_hash_map<size_t, size_t> ();
tbb::concurrent_hash_map<earray<uint>, earray<uint> *, earrayhash<uint> > Canonical::pattern_to_canonical2 = tbb::concurrent_hash_map<earray<uint>, earray<uint>*, earrayhash<uint>> ();
tbb::concurrent_unordered_set<earray<uint>, earrayhash<uint> > Canonical::canonical2 = tbb::concurrent_unordered_set<earray<uint>, earrayhash<uint>> ();
earray<uint> Canonical::nullphash = earray<uint> ();


//get canonical with embedding directly  
earray<uint>* Canonical::getHash2(BasicEmbedding &e) {
	return getHash2(e, -1);
}

//get canonical with embedding directly  
earray<uint>* Canonical::getHash2(BasicEmbedding &e, int wordId) {
	tbb::concurrent_hash_map<earray<uint>, earray<uint>*, earrayhash<uint>>::const_accessor a;
	earray<uint> connections = e.getConnections();

	int k = e.getNumVertices();
	if (wordId>=0) {
		earray<int> &vertices = e.getVertices();
		for (int i = 0; i < k; i++) {
			if (e.areWordsNeighbours(vertices[i], wordId)) {
				setKthBit(&(connections[i]),e.getNumVertices());
				setKthBit(&(connections[e.getNumVertices()]), i);
			}
		}
		k++;
	}

	return getHash2(connections, k);
}

earray<uint>* Canonical::getHash2(earray<uint> &connections, int k) {
        tbb::concurrent_hash_map<earray<uint>, earray<uint>*, earrayhash<uint>>::const_accessor a;
        earray<uint> *code;

        bool r = Canonical::pattern_to_canonical2.find(a, connections);       // creates by default if not exists, acquires lock
        if (r) {
                code = a->second;
                a.release();
        }
        else {
                a.release();
                tbb::concurrent_hash_map<earray<uint>, earray<uint>*, earrayhash<uint>>::accessor b;
                r = Canonical::pattern_to_canonical2.insert(b, connections);       // creates by default if not exists, acquires lock
                if (r) { // it means that it is new
                        bliss::Graph bg = getBlissGraph(connections, k);
                        bliss::Stats stats;
                        const unsigned int* perm = bg.canonical_form(stats, NULL, NULL);
                        bliss::Graph *bc = bg.permute(perm);
                        earray<uint>* phash = get_earray_hash(bc);
                        std::pair<tbb::concurrent_unordered_set<earray<uint>, earrayhash<uint>>::iterator, bool> can = canonical2.insert(*phash);
                        b->second = &(*can.first);
                        delete phash;
                        delete bc;
                }
                code = b->second;
                b.release();
        }
        return code;
}


//get canonical with embedding directly  
/*size_t Canonical::getHash(BasicEmbedding &e) {
	size_t naiveCode = getNaiveHash(e);
	tbb::concurrent_hash_map<size_t, size_t>::accessor a;
	bool r = Canonical::pattern_to_canonical.insert(a, naiveCode);       // creates by default if not exists, acquires lock

	if (r) { // it means that it is new
		bliss::Graph bg = getBlissGraph(e);
		bliss::Stats stats;
		const unsigned int* perm = bg.canonical_form(stats, NULL, NULL);
		bliss::Graph *bc = bg.permute(perm);
		a->second = bc->get_hash();
		delete bc;
		Canonical::misses++;
	}
	else {
		Canonical::hits++;
	}

	size_t code = a->second;
	a.release();

	if (Canonical::hits+Canonical::misses%10000==0) {
		std::cout << "Caninocal hashmap hits: " << Canonical::hits << " misses: " << Canonical::misses << std::endl;
	}

	return code;
}*/

//get canonical with embedding directly  
/*size_t Canonical::getHash(BasicEmbedding &e, int wordId) {
	size_t naiveCode = getNaiveHash(e, wordId);
	tbb::concurrent_hash_map<size_t, size_t>::accessor a;
	bool r = Canonical::pattern_to_canonical.insert(a, naiveCode);       // creates by default if not exists, acquires lock

	if (r) { // it means that it is new
		bliss::Graph bg = getBlissGraph(e, wordId);
		bliss::Stats stats;
		const unsigned int* perm = bg.canonical_form(stats, NULL, NULL);
		bliss::Graph *bc = bg.permute(perm);
		a->second = bc->get_hash();
		delete bc;
		Canonical::misses++;
	}
	else {
		Canonical::hits++;
	}

	size_t code = a->second;
	a.release();

	if (Canonical::hits+Canonical::misses%10000==0) {
		std::cout << "Caninocal hashmap hits: " << Canonical::hits << " misses: " << Canonical::misses << std::endl;
	}

	return code;
}*/

//get canonical with graph directly  
/*size_t Canonical::getHash(Graph &g) {
	size_t naiveCode = g.getNaiveCodeHashValue();
	tbb::concurrent_hash_map<size_t, size_t>::accessor a;
	bool r = Canonical::pattern_to_canonical.insert(a, naiveCode);       // creates by default if not exists, acquires lock

	if (r) { // it means that it is new
		bliss::Graph bg = g.getBlissGraph();
		bliss::Stats stats;
		const unsigned int* perm = bg.canonical_form(stats, NULL, NULL);
		bliss::Graph *bc = bg.permute(perm);
		a->second = bc->get_hash();
		delete bc;
		Canonical::misses++;
	}	
	else {
		Canonical::hits++;	
	}

	size_t code = a->second;
	a.release();

	if (Canonical::hits+Canonical::misses%10000==0) {
		std::cout << "Caninocal hashmap hits: " << Canonical::hits << " misses: " << Canonical::misses << std::endl;
	}	

	return code;
}*/

/*size_t Canonical::getHash(bliss::Graph &bg) {
//return Canonical::getHashScratch(bg);

size_t naiveCode = bg.get_hash();
tbb::concurrent_hash_map<size_t, size_t>::accessor a;
bool r = Canonical::pattern_to_canonical.insert(a, naiveCode);       // creates by default if not exists, acquires lock

PatternInfo *naiveInfo = NULL;
PatternInfo *canonicalInfo = NULL;

if (r) { // it means that it is new
bliss::Stats stats;
size_t N = bg.get_nof_vertices();

//naiveInfo->init(N);	
//canonicalInfo->init(N);	
naiveInfo = new PatternInfo(N);
canonicalInfo = new PatternInfo(N);

//structures to keep automorphic info
for (unsigned int i = 0; i < N ; i ++) {
canonicalInfo->array[i] = i;
}

//running canonical form func
//perm[i] is the vertex idx from the original graph keep in position i of the canonical one.
const unsigned int* perm = bg.canonical_form(stats, &report_aut, canonicalInfo->array);
memcpy(naiveInfo->array, perm, sizeof(unsigned int) * N);
bliss::Graph *bc = bg.permute(perm);
a->second = bc->get_hash();

//put parent to canonical format
unsigned int parentAux[N];
memcpy(parentAux, canonicalInfo->array, sizeof(unsigned int) * N);
for (unsigned int i = 0; i < N ; i ++) {
canonicalInfo->array[i] = parentAux[perm[i]];
}

//bg.write_dimacs(stdout);
//bc->write_dimacs(stdout);
//std::cout << "canonical perm: ["; 
//for (uint i = 0; i < N ; i ++) {
//	std::cout << " " << naiveInfo->array[i];
//}
//std::cout << " ]\n";
//std::cout << "parent: ["; 
//for (uint i = 0; i < N ; i ++) {
//	std::cout << " " << canonicalInfo->array[i];
//}
//std::cout << " ]\n";

delete bc;
}	

size_t code = a->second;
a.release();

// canonical code
return code;
}*/

/*unsigned int* Canonical::getPermutation(bliss::Graph &bg) {
  size_t naiveCode = bg.get_hash();
  tbb::concurrent_hash_map<size_t, PatternInfo*>::accessor b;
  bool r = Canonical::naives.find(b, naiveCode);       // creates by default if not exists, acquires lock
  if (r) { //means that was found
  unsigned int *array = b->second->array;
  b.release();
  return array;
  }
  else {
  b.release();
  getHash(bg);
  return getPermutation(bg);
  }

  }*/

/**
 * The hook function that prints the found automorphisms.
 * \a param must be a file descriptor (FILE *).
 */
/*void Canonical::report_aut(void* param, const unsigned int n, const unsigned int* aut) {
	//fprintf((FILE*)param, "Generator: ");
	//fprintf(stdout, "Generator: ");
	//bliss::print_permutation((FILE*)param, n, aut, 0);
	get_automorphism_map((unsigned int*)param, n, aut, 0);
	//std::cout << "[";
	//  for (int i = 0; i < n ; i ++) {
	//  std::cout << " " << aut[i];
	//  }
	//  std::cout << " ]\n";
	//fprintf(stdout, "\n");
}*/


/*void Canonical::get_automorphism_map(unsigned int* parent, const unsigned int N, const unsigned int* perm, const unsigned int offset) {
	assert(N > 0);
	assert(perm);

	for(unsigned int i = 0; i < N; i++) {
		unsigned int j = perm[i];
		if(j == i) continue;

		while(parent[i]>j) {
			parent[i] = j;
			j = parent[j];
		}
	}*/

	/*for(unsigned int i = 0; i < N; i++) {
	  unsigned int j = perm[i];
	  if(j == i)
	  continue;
	  bool is_first = true;
	  while(j != i) {
	  if(j < i) {
	  is_first = false;
	  break;
	  }
	  j = perm[j];
	  }
	  if(!is_first)
	  continue;
	  fprintf(stdout, "(%u,", i+offset);
	  j = perm[i];
	  while(j != i) {
	  if (parent[j] > i) parent[j] = i;
	  fprintf(stdout, "%u", j+offset);
	  j = perm[j];
	  if(j != i)
	  fprintf(stdout, ",");
	  }
	  fprintf(stdout, ")");
	  }*/

	/*std::cout << "#####canonical perm####: [";
	  for (int i = 0; i < N ; i ++) {
	  std::cout << " " << perm[i];
	  std::cout << " (" << parent[i] << ") ";

	  }
	  std::cout << " ]\n";*/
//}

/*void Canonical::merge_sets(int *parent, size_t N) {
  for (int i = 0; i < N; i++) {
  std::cout << "checking parent of id " << i << std::endl; 
  if (parent[i] == -1) {
  parent[i] = i;
  continue;
  }
  int j = parent[i];
//while(j != i) {
while(parent[j]!=j) {
j = parent[j];		
std::cout << "new parent " << j << std::endl;
}	
std::cout << "idx " << i << " parent " << j << std::endl;	
parent[i] = j;
}
}*/

/*bliss::Graph Canonical::getBlissGraph(BasicEmbedding &e) {
	//std::cout << "GBLISS CODE" << std::endl;
	bliss::Graph blissGraph;

	earray<int> &vertices = e.getVertices();
	earray<uint> &connections = e.getConnections();
	Graph *graph = e.getGraph();

	for (int i = 0; i < e.getNumVertices(); i++) {
		Node &vertex = graph->getNodeAt(vertices[i]);
		blissGraph.add_vertex(vertex.getLabel());
	}

	for (int i = 0; i < e.getNumVertices(); i++) {
		for (int j = (i+1); j < e.getNumVertices(); j++) {
			if (isKthBitSet(connections[i], j)) blissGraph.add_edge(i, j);
		}
	}
	return blissGraph;
}

bliss::Graph Canonical::getBlissGraph(BasicEmbedding &e, int wordId) {
	//std::cout << "GBLISS CODE" << std::endl;
	bliss::Graph blissGraph;

	earray<int> &vertices = e.getVertices();
	earray<uint> &connections = e.getConnections();
	Graph *graph = e.getGraph();

	for (int i = 0; i < e.getNumVertices(); i++) {
		Node &vertex = graph->getNodeAt(vertices[i]);
		blissGraph.add_vertex(vertex.getLabel());
	}

	Node &vertex = graph->getNodeAt(wordId);
	blissGraph.add_vertex(vertex.getLabel());

	for (int i = 0; i < e.getNumVertices(); i++) {
		for (int j = (i+1); j < e.getNumVertices(); j++) {
			if (isKthBitSet(connections[i], j)) blissGraph.add_edge(i, j);
		}
		if (graph->isNeighbor(vertices[i], wordId))
			blissGraph.add_edge(i, e.getNumVertices());
	}
	return blissGraph;
}*/

bliss::Graph Canonical::getBlissGraph(earray<uint> &connections, int k) {
        //std::cout << "GBLISS CODE" << std::endl;
        bliss::Graph blissGraph;

        for (int i = 0; i < k; i++) {
                blissGraph.add_vertex(0);
        }

        for (int i = 0; i < k; i++) {
                for (int j = (i+1); j < k; j++) {
                        if (isKthBitSet(connections[i], j)) blissGraph.add_edge(i, j);
                }
        }
        return blissGraph;
}

/*size_t Canonical::getNaiveHash(BasicEmbedding &e) {
	earray<uint> &connections = e.getConnections();

	size_t seed = 0;
	for (int i = 0; i < e.getNumVertices(); i++) {
		for (int j = (i+1); j < e.getNumVertices(); j++) {
			if (!isKthBitSet(connections[i], j)) continue;
			boost::hash_combine(seed, i * 2654435761);
			boost::hash_combine(seed, j * 2654435761);
		}
	}

	return seed;
}*/

/*size_t Canonical::getNaiveHash(BasicEmbedding &e, int wordId) {
	earray<int> &vertices = e.getVertices();
	earray<uint> &connections = e.getConnections();
	Graph *graph = e.getGraph();

	size_t seed = 0;
	for (int i = 0; i < e.getNumVertices(); i++) {
		for (int j = (i+1); j < e.getNumVertices(); j++) {
			if (!isKthBitSet(connections[i], j)) continue;
			boost::hash_combine(seed, i * 2654435761);
			boost::hash_combine(seed, j * 2654435761);
		}
		if (graph->isNeighbor(vertices[i], wordId)) {
			boost::hash_combine(seed, i * 2654435761);
			boost::hash_combine(seed, e.getNumVertices() * 2654435761);
		}
	}
	return seed;
}*/

earray<uint> *Canonical::get_earray_hash(bliss::Graph *bg){
	earray<uint> *code = new earray<uint>;	 
	code->fill(0);
	for(unsigned int i = 0; i < bg->get_nof_vertices(); i++) {
		bliss::Graph::Vertex &v = bg->vertices[i];
		for(std::vector<unsigned int>::const_iterator ei = v.edges.begin(); ei != v.edges.end(); ei++) {
			const unsigned int dest_i = *ei;
			if(dest_i < i)
			//if(dest_i == i)
				continue;
			setKthBit(&(code->data()[i]), dest_i);
			setKthBit(&(code->data()[dest_i]), i);
		}
	}
	return code;
}


std::string Canonical::getMotivoHash(earray<uint> *connections, int size, int padding) {
	std::string b;
	char a = 'A';
	char zero = '0';
	char one = '1';

	//std::cout << "Motivo hash input: " << *connections << std::endl;
	for (int i = 1; i < size; i++) {
		for (int j = 0; j < i; j++) {
			if (isKthBitSet((*connections)[i],j)) b.push_back(one);
			else b.push_back(zero);
		}
	}

	//std::cout << "b : " << b << std::endl;
	int n = 4-(int)b.size()%4;
	//std::cout << "#0's added: " << n << std::endl;
	for (int i = 0; i < n; i++) b.push_back(zero);
	//std::cout << "b : " << b << std::endl;

	std::string ord;
	for (int i = 0; i < (int) b.size()/4; i++) {
		//std::cout <<  b.substr(4*i,4)  << " " <<  (int) a << " " <<  std::stoi(b.substr(4*i,4*(i+1)), nullptr, 2) << std::endl;
		char c = ((int) a + std::stoi(b.substr(4*i,4), nullptr, 2));
		ord.push_back(c);
	}
	//std::cout << "ord : " << ord << std::endl;

	for (int i = ord.size(); i < padding; i++) ord.push_back(a);

	return ord;
}



