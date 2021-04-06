#include "EdgeInducedEmbedding.h"
#include "EmbeddingUtils.h"
#include <graph.hh>
#include <algorithm>


void EdgeInducedEmbedding::reset() {
	BasicEmbedding::reset();
	edges.clear();
	verticesMap.clear();
	//edgesSet.clear();
}

EdgeInducedEmbedding::EdgeInducedEmbedding(Graph *g, std::vector<int> ws):BasicEmbedding(g, ws) {
	edges.reserve(1000); 
	verticesMap.reserve(1000);
	//edgesSet.reserve(1000);
	for (int w : ws) addWord(w);
};

EdgeInducedEmbedding::EdgeInducedEmbedding(Graph *g):BasicEmbedding(g) {
	edges.reserve(1000); 
	verticesMap.reserve(1000);
	//edgesSet.reserve(1000);
};

EdgeInducedEmbedding::EdgeInducedEmbedding():BasicEmbedding() { 
	edges.reserve(1000); 
	verticesMap.reserve(1000);
	//edgesSet.reserve(1000);
};  

void EdgeInducedEmbedding::setFromEmbedding(EdgeInducedEmbedding other) {
}

/*void EdgeInducedEmbedding::copy(EdgeInducedEmbedding other) {
	graph = other.graph;
	edges = other.getEdges();
	edgesSet = other.getEdgesSet();
	verticesMap = other.getVerticesMap();
}*/

NeighborhoodSet EdgeInducedEmbedding::getWordNeighbors(int id) {
	NeighborhoodSet neighs;
	std::cout << "Function is not implemented!" << std::endl;
	exit(1);
}

//TODO
std::vector<int> &EdgeInducedEmbedding::getValidElementsForExpansionSorted(int vertexId) {
	std::vector<int> es;
	//return graph->getNeighborhoodIdxEdgeOfVertexAt(vertexId);
	return es;
}

//TODO
NeighborhoodSet &EdgeInducedEmbedding::getValidElementsForExpansion(int vertexId) {
	NeighborhoodSet es;
	//return graph->getNeighborhoodEdgeOfVertexAt(vertexId);
	return es;
}

    
//TODO
//std::unordered_map<int,std::pair<int,uint>> &EdgeInducedEmbedding::getValidElementsForExpansionWithConnectionPattern() {
std::unordered_map<int,uint> EdgeInducedEmbedding::getValidElementsForExpansionWithConnectionPattern() {
	return std::unordered_map<int,uint> ();
	//return neighborhoodMap;
}

std::unordered_set<int> EdgeInducedEmbedding::getValidElementsForContraction() {
	std::unordered_set<int> contractions;
	std::vector<bool> bridges = EmbeddingUtils::articulation(*this);

	for (int i = 0 ; i < (int)edges.size(); i++)
		if (!bridges[i])
			contractions.insert(edges[i]);	
	return contractions;
}

std::unordered_set<int> EdgeInducedEmbedding::getValidElementsForContractionWithWord(int wordId) {
	std::unordered_set<int> contractions;
	std::vector<bool> bridges = EmbeddingUtils::articulation(*this, wordId);

	for (int i = 0 ; i < (int)edges.size(); i++)
		if (!bridges[i])
			contractions.insert(edges[i]);	
	return contractions;
}

bool EdgeInducedEmbedding::areWordsNeighbours(int wordId1, int wordId2) {
	return graph->isNeighborEdge(wordId1, wordId2);
}

//TODO
std::vector<int> EdgeInducedEmbedding::getValidElementsForExpansionSorted() {
	return std::vector<int>();
}

std::unordered_set<int> EdgeInducedEmbedding::getValidElementsForExpansion() {
	std::unordered_set<int> expansions;
	if (edges.size() == 0) {
		//expansions.insert(-1);	
		//int vec[] = {-1};
		//return std::vector<int> (vec, vec + sizeof(vec) / sizeof(int)) ;
		expansions.insert(-1); 
	}
	for (std::unordered_map<int, int>::iterator it = verticesMap.begin(); it!=verticesMap.end(); it++) {
		NeighborhoodSet possibleExp = getValidElementsForExpansion(it->first);
		expansions.insert(possibleExp.begin(), possibleExp.end());
	}
	//remove edges already inserted
	for (int i = 0; i < (int)edges.size(); i++) {
		expansions.erase(edges[i]);
	}

	/*std::vector<int> bla (expansions.begin(), expansions.end());
	  std::vector<int> ble = getValidElementsForExpansion();

	  std::sort(bla.begin(), bla.end());
	  std::sort(ble.begin(), ble.end());
	  for (int i = 0; i < bla.size(); i++) {
	  if (bla[i]!=ble[i]) {
	  std::cout << "BLA != BLE\n";
	  exit(1);
	  }
	  }

	  std::cout << "before : " << bla.size() << " after " << ble.size() << std::endl;
	  return bla; 
	 */
	//return std::vector<int> (expansions.begin(), expansions.end());
	return expansions;
}

std::unordered_set<int> EdgeInducedEmbedding::getValidElementsForExpansionWith(std::unordered_set<int> &l) {
	std::unordered_set<int> expansions;
	if (edges.size() == 0) {
		//expansions.insert(-1);	
		//int vec[] = {-1};
		//return std::vector<int> (vec, vec + sizeof(vec) / sizeof(int)) ;
		expansions.insert(-1); 
	}
	for (std::unordered_map<int, int>::iterator it = verticesMap.begin(); it!=verticesMap.end(); it++) {
		NeighborhoodSet possibleExp = getValidElementsForExpansion(it->first);
		for (int j : possibleExp) 
			if (l.find(j) != l.end()) expansions.insert(j);
	}
	//remove edges already inserted
	for (int i = 0; i < (int)edges.size(); i++) {
		expansions.erase(edges[i]);
	}

	/*std::vector<int> bla (expansions.begin(), expansions.end());
	  std::vector<int> ble = getValidElementsForExpansion();

	  std::sort(bla.begin(), bla.end());
	  std::sort(ble.begin(), ble.end());
	  for (int i = 0; i < bla.size(); i++) {
	  if (bla[i]!=ble[i]) {
	  std::cout << "BLA != BLE\n";
	  exit(1);
	  }
	  }

	  std::cout << "before : " << bla.size() << " after " << ble.size() << std::endl;
	  return bla; 
	 */
	//return std::vector<int> (expansions.begin(), expansions.end());
	return expansions;
}

/**
 * Add word and update the number of vertices in this embedding.
 *
 * @param word
 */

void EdgeInducedEmbedding::addWord(int word) {
	edges.push_back(word);
	//edgesSet.insert(word);

	//update vertices
	Edge e = graph->getEdgeAt(word);
	int v = e.getFromNodeId();
	int u = e.getToNodeId();

	bool hasV = hasVertex(v);
	bool hasU = hasVertex(u);
	if (!hasV) {
		//add vertex
		//vertices.push_back(v);
		verticesMap.insert(std::make_pair(v, 1));

		//update extensions
		/*std::unordered_set<int> possibleExp = getValidElementsForExpansion(v);
		  for (std::unordered_set<int>::iterator it = possibleExp.begin(); it != possibleExp.end() ; it++) {
		  std::unordered_map<int, int>::iterator itExp = extensionsWords.find(*it);
		  if (itExp==extensionsWords.end()) {
		  extensionsWords.insert(std::make_pair(*it, 1));
		  }
		  else {
		  itExp->second+=1;
		  }
		  }*/
	}
	else {
		std::unordered_map<int, int>::iterator itVert = verticesMap.find(v);
		itVert->second+=1;
	}

	if (!hasU) {
		//add vertex
		//vertices.push_back(u);
		verticesMap.insert(std::make_pair(u, 1));

		//update extensions
		/*std::unordered_set<int> possibleExp = getValidElementsForExpansion(u);
		  for (std::unordered_set<int>::iterator it = possibleExp.begin(); it != possibleExp.end() ; it++) {
		  std::unordered_map<int, int>::iterator itExp = extensionsWords.find(*it);
		  if (itExp==extensionsWords.end()) {
		  extensionsWords.insert(std::make_pair(*it, 1));
		  }
		  else {
		  itExp->second+=1;
		  }
		  }*/
	}
	else {
		std::unordered_map<int, int>::iterator itVert = verticesMap.find(u);
		itVert->second+=1;
	}
}

void EdgeInducedEmbedding::replaceWord(int word1, int word2) {
	std::cout << "PROBLEM: replaceWord for edge induced embeddings is not implemented!" << std::endl;
	exit(1);
}

void EdgeInducedEmbedding::removeLastWord() {
	if (edges.size()<=1) {
		edges.clear();
		//edgesSet.clear();
		verticesMap.clear();
		return;
	}
	int word = edges.back();
	Edge e = graph->getEdgeAt(word);
	edges.pop_back();
	//edgesSet.erase(word);

	int v = e.getFromNodeId();
	int u = e.getToNodeId();

	//confirm that removed edge is the only one with these nodes
	bool hasV = false;
	bool hasU = false;

	std::unordered_map<int, int>::iterator itVert = verticesMap.find(v);
	//if (itVert==verticesMap.end()) {
	//}
	if (itVert->second>1) {
		hasV = true;
		itVert->second--;
	}
	else if (itVert->second == 1) 
		verticesMap.erase(itVert); 
	else {
		std::cout << "problem node not found!" << std::endl;
		exit(1);
	}

	itVert = verticesMap.find(u);
	//if (itVert==verticesMap.end()) {
	//}
	if (itVert->second>1) {
		hasU= true;
		itVert->second--;
	}
	else if (itVert->second == 1) 
		verticesMap.erase(itVert); 
	else {
		std::cout << "problem node not found!" << std::endl;
		exit(1);
	}


	if (!hasV && !hasU) {
		std::cout << "error : an edge can not remove two vertices!\n";
		std::cout << "edge " << word << " vertices " << u << " " << v  <<  "\n";
		this->print();
		exit(1);
	}

}

void EdgeInducedEmbedding::removeWord(int word) {
	//std::assert(pos == -1);

	if (edges.size()<=1) {
		edges.clear();
		//edgesSet.clear();
		//vertices.clear();
		verticesMap.clear();
		return;
	}

	//erase edge	
	bool edgeFound = false;
	std::vector<int>::iterator it = edges.begin(); 
	while (it!=edges.end()) {
		//std::cout << *it << " " << word << std::endl;
		if (*it == word) {
			edgeFound = true;
			edges.erase(it);
			//edgesSet.erase(word);
			break;
		}
		it++;

	}

	if (!edgeFound) {
		std::cout << "problem edge not found!" << std::endl;
		exit(1);
	}

	Edge e = graph->getEdgeAt(word);
	int v = e.getFromNodeId();
	int u = e.getToNodeId();

	//confirm that removed edge is the only one with these nodes
	bool hasV = false;
	bool hasU = false;
	/*for (int i = 0; i < edges.size(); i++) {
	  Edge remainEdge = graph->getEdgeAt(edges[i]);
	  if (remainEdge.hasVertex(v)) {
	  hasV = true;
	  }
	  if (remainEdge.hasVertex(u)) {
	  hasU = true;
	  }
	  if (hasV && hasU) break;
	  }*/
	std::unordered_map<int, int>::iterator itVert = verticesMap.find(v);
	//if (itVert==verticesMap.end()) {
	//}
	if (itVert->second>1) {
		hasV = true;
		itVert->second--;
	}
	else if (itVert->second == 1) 
		verticesMap.erase(itVert); 
	else {
		std::cout << "problem node not found!" << std::endl;
		exit(1);
	}



	itVert = verticesMap.find(u);
	//if (itVert==verticesMap.end()) {
	//}
	if (itVert->second>1) {
		hasU= true;
		itVert->second--;
	}
	else if (itVert->second == 1) 
		verticesMap.erase(itVert); 
	else {
		std::cout << "problem node not found!" << std::endl;
		exit(1);
	}


	if (!hasV && !hasU) {
		std::cout << "error : an edge can not remove two vertices!\n";
		std::cout << "edge " << word << " vertices " << u << " " << v  <<  "\n";
		this->print();
		exit(1);
	}


	//std::cout << "#### extensions size: " << extensionsWords.size() << std::endl;
	//for (std::unordered_map<int,int>::iterator it = extensionsWords.begin(); it != extensionsWords.end() ; it++) 
	//		std::cout << it->first << ":" << it->second << "\t";
	//std::cout << std::endl;

	//update extensionsWords
	/*if (!hasV) {
	  std::unordered_set<int> possibleExp = getValidElementsForExpansion(v);
	  for (std::unordered_set<int>::iterator it = possibleExp.begin(); it != possibleExp.end() ; it++) {
	  std::unordered_map<int, int>::iterator itExp = extensionsWords.find(*it);
	  if (itExp==extensionsWords.end()) {
	  std::cout << "error! embedding problem! removeWord\n";
	  std::cout << "removed node "<< v << " not found " << *it << "\n";
	  exit(1);
	  }
	  else if (itExp->second>1){
	  itExp->second--;
	  }
	  else {
	//std::cout << "removing extension " << itExp->first << std::endl;
	extensionsWords.erase(itExp);
	}
	}
	}
	if (!hasU) {
	std::unordered_set<int> possibleExp = getValidElementsForExpansion(u);
	for (std::unordered_set<int>::iterator it = possibleExp.begin(); it != possibleExp.end() ; it++) {
	std::unordered_map<int, int>::iterator itExp = extensionsWords.find(*it);
	if (itExp==extensionsWords.end()) {
	std::cout << "error! embedding problem! removeWord";
	std::cout << "removed node "<< u << " not found " << *it << "\n";
	exit(1);
	}
	else if (itExp->second>1){
	itExp->second--;
	}
	else {
	extensionsWords.erase(itExp);
	}
	}
	}*/

	//std::cout << "NEW: ";
	//this->print(); 
}

int EdgeInducedEmbedding::getTotalNumWords(){
	return graph->getNumberOfEdges();
}

std::vector<int> &EdgeInducedEmbedding::getWords() {
	return getEdges();
}

int EdgeInducedEmbedding::getNumWords() {
	return getNumEdges();
}

int EdgeInducedEmbedding::getRandomWordBiased() {
	return graph->getRandomEdgeBiased();
}

