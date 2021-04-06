import sys
import networkx as nx
from func_utils import *

## Adds a new node that is connected to all others

## Input file format:
## VertexId Label Neigh1 Neigh2 ....
## Output file format:
## Origin Destini
inputFile = sys.argv[1]
outputFile = sys.argv[2]

G = read_graph_gph(inputFile)

#for value  in nx.triangles(G).values():
#	print value
#	print key, value


core = set()
cores = nx.core_number(G)
#print cores
for node,coren  in cores.iteritems():
	if coren == 1:
		core.add(node)	
	print "core", node, coren


print core
nodes = list(G.nodes())
#remove nodes out of core
for node in nodes:
	if node not in core:	
		G.remove_node(node)
#		print "is NOT in this set"
#	else: 
#		print "is in this set"

write_graph_gph(G, outputFile)
print "Graph all the valid nodes:"
#nx.info(G)
print "=number of nodes:", nx.number_of_nodes(G)
print "=number of edges:", nx.number_of_edges(G)
G = max(nx.connected_component_subgraphs(G), key=len)
print "Graph max component:"
#nx.info(G)
print "=number of nodes:", nx.number_of_nodes(G)
print "=number of edges:", nx.number_of_edges(G)
