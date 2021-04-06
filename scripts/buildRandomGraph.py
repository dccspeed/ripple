import sys
import networkx as nx
from func_utils import *

## Adds a new node that is connected to all others

## Input file format:
## VertexId Label Neigh1 Neigh2 ....
## Output file format:
## Origin Destini
n = int(sys.argv[1])
p = float(sys.argv[2])
outputFile = sys.argv[3]

G=nx.erdos_renyi_graph(n, p)
G = max(nx.connected_component_subgraphs(G), key=len)
G = sort_graph_by_degree(G)
write_graph_gph(G, outputFile, label=False);

