import sys
import networkx as nx
import igraph as ig

def read_graph_snap_igraph(inputFile):
	G = ig.Graph()
	with open(inputFile) as file:
		for line in file.readlines():
			line = line.rstrip()
			if not line.startswith("#"):
				line = line.rsplit();
				source = line[0]
				dest = line[1]
				if (int(source)>=G.vcount()):
					G.add_vertices(int(source)-G.vcount()+1)
				if (int(dest)>=G.vcount()):
					G.add_vertices(int(dest)-G.vcount()+1)
				#G.add_vertex(name=source)
				#G.add_vertex(name=dest)
				G.add_edges([(int(source), int(dest))])
				print("add:", int(source), int(dest), G.vcount())
	return G

def read_graph_snap(inputFile):
	G = nx.Graph()
	with open(inputFile) as file:
		for line in file.readlines():
			line = line.rstrip()
			if not line.startswith("#"):
				line = line.rsplit();
				source = int(line[0])
				dest = int(line[1])
				G.add_node(source);
				G.add_node(dest);
				G.add_edge(source, dest)
	return G

def read_graph_gph(inputFile):
	G = nx.Graph()
	with open(inputFile) as file:
		for line in file.readlines():
			line = line.rstrip().split(" ")
			type = line[0]
			if(type == 'v'):
				node = int(line[1])
				label = data = line[2:]
				G.add_node(node, vlabel = label)
			if(type == 'e'):
				source = int(line[1])
				dest = int(line[2])
				lab = line[3]
				G.add_edge(source, dest, elabel = lab)
	file.close()
	return G

def write_graph_gph(G, outputFile, label=True):
	with open(outputFile, 'w') as file:
		# Print label of each node
		for (node, data) in (G.nodes(data = True)):
			#output.write("v " + str(node) + " " + str(G.node[node]['label']) + "\n")
			file.write("v " + str(node))
			if (label == True):
				 for dat in data['vlabel']:
				    file.write(" {}".format(dat))	
			else:
				file.write(" 0")
			file.write('\n')
		# Print edges
		for (source, dest, data) in G.edges(data = True):
			file.write("e " + str(source)  + " " + str(dest))
			if (label == True):
				file.write(" {}".format(data['elabel']))
			else:
				file.write(" 0")
			file.write('\n')
	file.close()

def write_graph_gph_igraph(G, outputFile, label=True):
        with open(outputFile, 'w') as file:
                # Print label of each node
                for node in G.vs:
                        file.write("v " + str(node.index))
                        file.write(" 0")
                        file.write('\n')
                # Print edges
                for (source, dest) in G.get_edgelist():
                        file.write("e " + str(source)  + " " + str(dest))
                        file.write(" 0")
                        file.write('\n')
        file.close()

def sort_graph_by_degree(G, reverse=False):
	new_G = nx.Graph()
	new_node=0
	mapids = {}
	#sorted_degree_nodes = sorted(G.degree().items(), key=lambda x: x[1], reverse=reverse)
	degree_nodes = dict(G.degree())
	sorted_degree_nodes = sorted(degree_nodes.items(), key=lambda x: x[1], reverse=reverse)
	for node, val in sorted_degree_nodes:
		mapids[node]=new_node
		#print "map ", node, val, new_node
		new_node=new_node+1
	
	for (node, data) in (G.nodes(data = True)):
		if data: 
			new_G.add_node(mapids[node], data)
		else :
			new_G.add_node(mapids[node])
	
	for (source, dest, data) in G.edges(data = True):
		if data:
			new_G.add_edge(mapids[source], mapids[dest], data)
		else:
			new_G.add_edge(mapids[source], mapids[dest])
	
	return new_G


def sort_graph_by_pagerank(G, reverse=False, alpha=0.85):
	new_G = nx.Graph()
	new_node=0
	mapids = {}
	pr = nx.pagerank(G, alpha)
	
	sorted_pr_nodes = sorted(pr.items(), key=lambda x: x[1], reverse=reverse)
	#print sorted_pr_nodes
	for node, val in sorted_pr_nodes:
		mapids[node]=new_node
		#print "map ", node, val, new_node
		new_node=new_node+1
	
	for (node, data) in (G.nodes(data = True)):
		new_G.add_node(mapids[node], data)
	
	for (source, dest, data) in G.edges(data = True):
		new_G.add_edge(mapids[source], mapids[dest], data)

	return new_G


def write_graph_motivo(G, outputFile, label=True):
        with open(outputFile, 'w') as file:
            # Print #nodes and number of edges
            file.write(str(nx.number_of_nodes(G))  + " " + str(nx.number_of_edges(G)) + '\n')
            for (node, data) in (G.nodes(data = True)):
                edges = nx.neighbors(G, node)
                file.write(str(len(list(edges))))
                for neigh in sorted(G.neighbors(node)):
                    file.write(" " + str(neigh))
                file.write('\n')
                #file.write(len(edges) + '\n')
        file.close()


#G = read_graph_gph(inputFile)
#G = read_graph_snap(inputFile)
#G = nx.k_core(G, 2);
#pr = nx.pagerank(G, alpha=0.9)
#G = max(nx.connected_component_subgraphs(G), key=len)
#G = sort_graph_by_degree(G)
