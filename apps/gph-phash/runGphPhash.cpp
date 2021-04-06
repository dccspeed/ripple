#include <vector>    // std::random_shuffle
#include <algorithm>    // std::random_shuffle
#include <gsl/gsl_math.h>
#include <gsl/gsl_cdf.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <thread>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/program_options.hpp> 
#include <limits>
#include "logging.h"
#include "graphSetReader.h"
#include "graph.h"
#include "utils.h"
#include "VertexInducedEmbedding.h"
#include "canonical.h"

int numthreads=1;

namespace 
{ 
    const size_t ERROR_IN_COMMAND_LINE = 1; 
    const size_t SUCCESS = 0; 
    const size_t ERROR_UNHANDLED_EXCEPTION = 2; 

} // namespace

int main(int argc, char **argv) {

	//parameters
	std::string input, output, config, emb_string;
        boost::log::trivial::severity_level logSeverity = boost::log::trivial::info;
	try 
	{ 
		/** Define and parse the program options 
		 */ 
		namespace po = boost::program_options; 
		po::options_description desc("Options"); 
		desc.add_options() 
			("help", "Print help messages") 
			("input,i", po::value<std::string>()->required(), "Input file name.") 
			("loglevel,l", po::value<boost::log::trivial::severity_level>(), "Log level to output")
			("config,c", po::value<std::string>(), "Configuration file");

		po::variables_map vm; 
		try 
		{ 
			po::store(po::parse_command_line(argc, argv, desc),  
					vm); // can throw 

			/** --help option 
			 */ 
			if ( vm.count("help")) { 
				std::cout << "Basic Command Line Parameter App" << std::endl 
					<< desc << std::endl; 
				return SUCCESS; 
			} 
			if ( vm.count("input")) {
				input = vm["input"].as<std::string>();
			}
			if ( vm.count("loglevel")) {
				logSeverity = vm["loglevel"].as<boost::log::trivial::severity_level>();
			}
			if ( vm.count("config")) {
				config = vm["config"].as<std::string>();
			}

			po::notify(vm); // throws on error, so do after help in case 

			//std::cout << "parameters : "  << input << " " << espace << " " << prob << std::endl;
			po::notify(vm); // throws on error, so do after help in case 
			// there are any problems 
		} 
		catch(po::error& e) 
		{ 
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
			std::cerr << desc << std::endl; 
			return ERROR_IN_COMMAND_LINE; 
		} 

		// application code here // 
	}

	catch(std::exception& e) 
	{ 
		std::cerr << "Unhandled Exception reached the top of main: " 
			<< e.what() << ", application will now exit" << std::endl; 
		return ERROR_UNHANDLED_EXCEPTION; 

	} 
        
        // set log level
        init_logging(logSeverity);

	//output file.
	std::ofstream ofs (output, std::ofstream::out);

	/* initialize random seed: */
	srand (time(NULL));

	GraphSetReader gphSetReader;
	gphSetReader.openData(input);

	uint graphId = 0;
	//std::cout << "blah\n";
	std::pair<Graph, bool> g = gphSetReader.readGraph();
	//std::cout << "creatting neighborhood vertex index...\n";
	g.first.createNeighborhoodIndex();
	g.first.setId(graphId);
	//g.first.printResume();

	
	//create embedding

	VertexInducedEmbedding vi(&g.first);	
	
	for (int i = 0; i < g.first.getNumberOfNodes(); i++) 
		vi.addWord(i);

	/*VertexInducedEmbedding e(&g.first);
	e.loadFromString(emb_string);
	std::cout << e << std::endl;
	
	for (int i : e.getEdges()) {
		Edge &edge = g.first.getEdgeAt(i);
		std::cout << edge << std::endl;
	}*/

	earray<uint> &connections = vi.getConnections();
	for (int i  = 0 ; i  < vi.getNumVertices(); i++) {
		for (int j  = 0 ; j  < vi.getNumVertices(); j++) {
			if (isKthBitSet(connections[i], j)) std::cout << "1 ";
			else std::cout <<  "0 ";
		}
		std::cout << std::endl;
	}

	earray<uint> *pattern = Canonical::getHash2(vi);
	std::cout << "Pattern size: " << vi.getNumVertices() << " Pattern hash: " << Canonical::getHash(vi) << " " << Canonical::getMotivoHash(pattern, vi.getNumVertices(), 30) << std::endl;


	gphSetReader.closeData();


	return 0;
}
