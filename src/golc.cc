// Communication
#include <graybat.hpp>

// STL
#include <iostream>   /* std::cout */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <cmath>      /* sqrt */
#include <cstdlib>    /* atoi */
#include <utility>    /* std::pair, std::make_pair */

// Boost
#include <boost/iterator/permutation_iterator.hpp> /* boost::make_permutation_iterator*/

typedef unsigned Cell;


struct Mesh : public graybat::graphPolicy::SimpleProperty{
    Mesh() : Mesh(0) {}
    Mesh(ID id) : SimpleProperty(id),
		  core(9,0),
		  border(14,0),
		  coordinates(0,0){
      // unsigned random = rand() % 10000;
      // if(random < 3125){
      // 	isAlive[0] = 1;
      // }

    }
	
    std::vector<Cell> core;
    std::vector<Cell> border;
    std::pair<unsigned, unsigned> coordinates;
    //unsigned aliveNeighbors;


};

struct Link : public graybat::graphPolicy::SimpleProperty{
    Link() : Link(0){}
    Link(ID id) : SimpleProperty(id){

    }

    
    typedef std::vector<Cell>::iterator it;
    //typedef std::array<unsigned, 3> xit;
    
    boost::permutation_iterator<it, it> src_begin;
    boost::permutation_iterator<it, it> src_end;

    boost::permutation_iterator<it, it> dest_begin;
    boost::permutation_iterator<it, it> dest_end;


};




void printGolDomain(const std::vector<unsigned> domain, const unsigned width, const unsigned height, const unsigned generation){
    for(unsigned i = 0; i < domain.size(); ++i){
	if((i % (width)) == 0){
	    std::cerr << std::endl;
	}

	if(domain.at(i)){
	  std::cerr << "#";
	}
	else {
	    std::cerr << " ";
	}

    }
    std::cerr << "Generation: " << generation << std::endl;
    for(unsigned i = 0; i < height+1; ++i){
	std::cerr << "\033[F";
    }

}


template <class T_Cell>
void updateState(std::vector<T_Cell> &cells){
    for(T_Cell &cell : cells){
	updateState(cell);

    }

}


template <class T_Cell>
void updateState(T_Cell &cell){
    switch(cell.aliveNeighbors){

    case 0:
    case 1:
	cell.isAlive[0] = 0;
	break;

    case 2:
	cell.isAlive[0] = cell.isAlive[0];
	break;
	    
    case 3: 
	cell.isAlive[0] = 1;
	break;

    default: 
	cell.isAlive[0] = 0;
	break;

    };

}


int gol(const unsigned nCells, const unsigned nTimeSteps ) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // CommunicationPolicy
    typedef graybat::communicationPolicy::BMPI    CP;
    
    // GraphPolicy
    typedef graybat::graphPolicy::BGL<Mesh, Link> GP;
    
    // Cave
    typedef graybat::Cave<CP, GP>   MyCave;
    typedef typename MyCave::Event  Event;
    typedef typename MyCave::Vertex Vertex;
    typedef typename MyCave::Edge   Edge;

    /***************************************************************************
     * Initialize Communication
     ****************************************************************************/
    //Create Graph
    // const unsigned height = sqrt(nCells);
    // const unsigned width  = height;

    const unsigned height = 1;
    const unsigned width  = 2;
    
    // Create GoL Graph
    MyCave cave(graybat::pattern::Grid(height, width));
    
    // Distribute vertices
    cave.distribute(graybat::mapping::Random(1));

    // Assign coordinates to grid vertices
    for(Vertex v: cave.getVertices()){
	unsigned id = v.id;

	unsigned x = id % width;
	unsigned y = (id / width);

	v.coordinates = std::make_pair(x,y);

	cave.setVertex(v);
    }

    // Precompute permutation_iterator
    for(Vertex &v: cave.getVertices()){
	for(auto link : cave.getOutEdges(v)){
	    Vertex destVertex = link.first;
	    Edge   destEdge   = link.second;

	    std::vector<unsigned> srcIndices {{2,5,8}};

	    destEdge.src_begin = boost::make_permutation_iterator(srcIndices.begin(), v.core.begin());
	    destEdge.src_end = boost::make_permutation_iterator(srcIndices.end(), v.core.end());



	}

	for(auto &link : cave.getInEdges(v)){
	    Vertex& srcVertex = link.first;
	    Edge&   srcEdge   = link.second;

	    std::vector<unsigned> destIndices {{5,6,7}};

	    srcEdge.dest_begin = boost::make_permutation_iterator(destIndices.begin(), srcVertex.border.begin());
	    srcEdge.dest_end   = boost::make_permutation_iterator(destIndices.end(), srcVertex.border.end());
	    

	}
	
    }

    
    /***************************************************************************
     * Run Simulation
     ****************************************************************************/
     std::vector<Event> events;   
     std::vector<unsigned> golDomain(cave.getVertices().size(), 0); 
     const Vertex root = cave.getVertex(0);


     // Simulate life 
      for(unsigned timestep = 0; timestep < nTimeSteps; ++timestep){

     // 	// Print life field by owner of vertex 0
     // 	if(cave.peerHostsVertex(root)){
     // 	    printGolDomain(golDomain, width, height, timestep);
     // 	}
	
	  // Send state to neighbor cells
	  for(Vertex &v : cave.hostedVertices){
	      for(auto link : cave.getOutEdges(v)){
		  Vertex destVertex = link.first;
		  Edge   destEdge   = link.second;

		  
		  std::array<unsigned, 3> send{{1,1,1}};

		  std::cout << destVertex.id << " " << destVertex.coordinates.first << " " << destVertex.coordinates.second << std::endl;
		
		  events.push_back(cave.asyncSend(destVertex, destEdge, send));
	      }
	  }

     	// Recv state from neighbor cells
     	for(Vertex &v : cave.hostedVertices){
     	    for(auto link : cave.getInEdges(v)){
     		Vertex srcVertex = link.first;
     		Edge   srcEdge   = link.second;
		std::array<unsigned, 3> recv{{0,0,0}};
     		cave.recv(srcVertex, srcEdge, recv);
		std::array<unsigned, 3> indices{{5,6,7}};

		auto it_begin = boost::make_permutation_iterator(srcVertex.border.begin(), indices.begin());
		std::copy(recv.begin(), recv.end(), it_begin);

		for(unsigned u: srcVertex.border){
		    std::cout << u << " ";
		}
		std::cout << std::endl;
		
     		//if(srcVertex.isAlive[0]) v.aliveNeighbors++;
     	    }
     	}

     // 	// Wait to finish events
     // 	for(unsigned i = 0; i < events.size(); ++i){
     // 	    events.back().wait();
     // 	    events.pop_back();
     // 	}

     // 	// Calculate state for next generation
     // 	updateState(cave.hostedVertices);

     // 	// Gather state by vertex with id = 0
     // 	for(Vertex &v: cave.hostedVertices){
     // 	    v.aliveNeighbors = 0;
     // 	    cave.gather(root, v, v.isAlive, golDomain, true);
     // 	}
	
      }
    
    return 0;

}

int main(int argc, char** argv){

    if(argc < 3){
	std::cout << "Usage ./GoL [nCells] [nTimeteps]" << std::endl;
	return 0;
    }

    const unsigned nCells    = atoi(argv[1]);
    const unsigned nTimeSteps = atoi(argv[2]);


    gol(nCells, nTimeSteps);


    return 0;
}
