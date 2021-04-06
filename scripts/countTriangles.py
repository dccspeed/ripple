import sys
import networkx as nx
from func_utils import *

## Adds a new node that is connected to all others

## Input file format:
## VertexId Label Neigh1 Neigh2 ....
## Output file format:
## Origin Destini
inputFile = sys.argv[1]
#outputFile = sys.argv[2]

G = read_graph_gph(inputFile)

#for value  in nx.triangles(G).values():
#	print value
#	print key, value
print sum(nx.triangles(G).values())/3
#G = max(nx.connected_component_subgraphs(G), key=len)
#print sum(nx.triangles(G).values())/3
#write_graph_gph(G, outputFile)
