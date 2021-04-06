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

#G = read_graph_gph(inputFile)
G = read_graph_gph(inputFile)

#G = sort_graph_by_pagerank(G)
write_graph_motivo(G, outputFile, label=False);

