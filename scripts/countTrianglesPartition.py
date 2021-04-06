import sys
import networkx as nx
import numpy as np

from func_utils import *


## Adds a new node that is connected to all others

## Input file format:
inputFile = sys.argv[1]
nodesstr = sys.argv[2]
outputFile = sys.argv[3]

G = read_graph_gph(inputFile)

#for value  in nx.triangles(G).values():
#	print value
#	print key, value
#print sum(nx.triangles(G).values())/3

print "initial nodes: ", nodesstr
nodes = nodesstr.split(",")

bnodes = np.full(nx.number_of_nodes(G), False, dtype=bool)

newG = nx.Graph()
newG.add_nodes_from(G)

for k in nodes:
	node=int(k)
	bnodes[node] = True
	#for src,dest in G.edges(node):
		#print src, dest
		#bnodes[dest]=True

covered=0
pid=0
while covered!=nx.number_of_edges(G):	
	newBnodes = np.full(nx.number_of_nodes(G), False, dtype=bool)

	numInserted=0
	for node in range(0,nx.number_of_nodes(G)):
		if bnodes[node]==True:
			numInserted=numInserted+1
			#print "checking edges of ", node, "#neighbors: ", len(G.edges(node));
			for src,dest in G.edges(node):
				if bnodes[src]==True and bnodes[dest]==True and newG.has_edge(src,dest)==False:
					newG.add_edge(src,dest);
	


	#counting triangles in this partition 	
	count=sum(nx.triangles(newG).values())/3
	print "count: ", count, "numEdges: ", nx.number_of_edges(newG), "numNodes: ", numInserted
	covered=nx.number_of_edges(newG)

	#expand graph	
	for node in range(0,nx.number_of_nodes(G)):
		if bnodes[node]==True:
			#print "adding neighbors of ", node, "#neighbors: ", len(G.edges(node));
			for src,dest in G.edges(node):
				 if bnodes[dest]==False:
					 newBnodes[dest]=True
	
	for node in range(0,nx.number_of_nodes(G)):
		if newBnodes[node] == True:
			bnodes[node] = True
	
	#write down partition graph	
	write_graph_gph(newG, outputFile+"partition"+str(pid), False)
	pid = pid + 1
	

	
#for n in sorted(G.neighbors(node)): 

