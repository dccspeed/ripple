import sys
#import networkx as nx
from func_utils import *

## Adds a new node that is connected to all others

## Input file format:
## VertexId Label Neigh1 Neigh2 ....
## Output file format:
## Origin Destini
inputFile = sys.argv[1]
outputFile = sys.argv[2]

#G = read_graph_gph(inputFile)

#G = nx.k_core(G, 2);
#G = max(nx.connected_component_subgraphs(G), key=len)

#remove nodes with one edge recursively
#print "graph size ", G.number_of_nodes()
#while True:
#	removeNodes=[]
#	for n,d in G.degree():
#		if d < 2:
#			#print "appending node ", n
#			removeNodes.append(n)
#	G.remove_nodes_from(removeNodes)
#	print "graph size ", G.number_of_nodes()
#	if len(removeNodes)==0:
#		break

#G = sort_graph_by_degree(G)
#G = sort_graph_by_pagerank(G)
#write_graph_gph(G, outputFile, label=False);

G = read_graph_snap_igraph(inputFile)
G = G.clusters().giant()
write_graph_gph_igraph(G, outputFile)
