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

#define WIDTH 2

typedef unsigned Cell;


struct Mesh : public graybat::graphPolicy::SimpleProperty{
    Mesh() : Mesh(0) {}
    Mesh(ID id) : SimpleProperty(id),
		  core{{0,1,2,3,4,5,6,7,8}},
		  border(16,0){
      // unsigned random = rand() % 10000;
      // if(random < 3125){
      // 	isAlive[0] = 1;
      // }

    }
	
    std::vector<Cell> core;
    std::vector<Cell> border;
    std::pair<unsigned, unsigned> coordinates(){
	unsigned x = id % WIDTH;
	unsigned y = (id / WIDTH);

	return std::make_pair(x,y);
    }
    
    //unsigned aliveNeighbors;


};

struct Link : public graybat::graphPolicy::SimpleProperty{
    Link() : Link(0){}
    Link(ID id) : SimpleProperty(id){

    }

    typedef std::vector<Cell>::iterator it;

    std::vector<Cell> srcIndices;
    std::vector<Cell> destIndices;

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

typedef std::vector<Cell>::iterator it2;

boost::permutation_iterator<it2, it2> begin;
boost::permutation_iterator<it2, it2> end;


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

    const unsigned height = 2;
    const unsigned width  = 2;
    
    // Create GoL Graph
    MyCave cave(graybat::pattern::GridDiagonal(height, width));
    
    // Distribute vertices
    cave.distribute(graybat::mapping::Roundrobin());

    // Think of some good way to
    // create connections between memories
    // of vertices --> DataChannels
    // Precompute permutation_iterator
    for(Vertex v: cave.getVertices()){
	unsigned x = v.coordinates().first;
	unsigned y = v.coordinates().second;

	// std::cout << "(" << x << "," << y << ")" << std::endl;

	
	for(auto link : cave.getOutEdges(v)){
	    Vertex destVertex = link.first.id;
	    Edge&   destEdge   = *(cave.getEdge2(v, destVertex));

	    unsigned xDest = destVertex.coordinates().first;
	    unsigned yDest = destVertex.coordinates().second;

	    // std::cout << "dest " << destVertex.id << "(" << xDest << "," << yDest << ")" << std::endl;
	    
	    if(x == xDest and y < yDest) // up
		destEdge.srcIndices = std::vector<Cell>{{0,1,2}};
	    if(x < xDest and y < yDest) // up right
		destEdge.srcIndices = std::vector<Cell>{{2}};
	    if(x < xDest and y == yDest) // right
		destEdge.srcIndices = std::vector<Cell>{{2,5,8}};
	    if(x < xDest and y > yDest) // down right
		destEdge.srcIndices = std::vector<Cell>{{8}};
	    if(x == xDest and y > yDest) // down
		destEdge.srcIndices = std::vector<Cell>{{6,7,8}};
	    if(x > xDest and y > yDest) // down left
		destEdge.srcIndices = std::vector<Cell>{{6}};
	    if(x > xDest and y == yDest) // left
		destEdge.srcIndices = std::vector<Cell>{{0,3,6}};
	    if(x > xDest and y < yDest) // up left
		destEdge.srcIndices = std::vector<Cell>{{0}};


	}

	for(auto &link : cave.getInEdges(v)){
	    Vertex srcVertex = link.first.id;
	    Edge&   srcEdge  = *(cave.getEdge2(srcVertex, v));

	    unsigned xSrc = srcVertex.coordinates().first;
	    unsigned ySrc = srcVertex.coordinates().second;

	    // std::cout << "src " << srcVertex.id << "(" << xSrc << "," << ySrc << ")" << std::endl;
	    
	    if(x == xSrc and y < ySrc) // up
		srcEdge.destIndices = std::vector<Cell>{{0,1,2}};
	    if(x < xSrc and y < ySrc) // up right
		srcEdge.destIndices = std::vector<Cell>{{3}};
	    if(x < xSrc and y == ySrc) // right
		srcEdge.destIndices = std::vector<Cell>{{4,5,6}};
	    if(x < xSrc and y > ySrc) // down right
		srcEdge.destIndices = std::vector<Cell>{{7}};
	    if(x == xSrc and y > ySrc) // down
		srcEdge.destIndices = std::vector<Cell>{{10,9,8}};
	    if(x > xSrc and y > ySrc) // down left
		srcEdge.destIndices = std::vector<Cell>{{11}};
	    if(x > xSrc and y == ySrc) // left
		srcEdge.destIndices = std::vector<Cell>{{14,13,12}};
	    if(x > xSrc and y < ySrc) // up left
		srcEdge.destIndices = std::vector<Cell>{{15}};

	}
	
    }

    
    /***************************************************************************
     * Run Simulation
     ****************************************************************************/
     std::vector<Event> events;   
     std::vector<unsigned> golDomain(cave.getVertices().size() * cave.getVertex(0).core.size(), 0); 
     const Vertex root = cave.getVertex(0);


     // Simulate life 
      for(unsigned timestep = 0; timestep < nTimeSteps; ++timestep){

     	// // Print life field by owner of vertex 0
     	// if(cave.peerHostsVertex(root)){
     	//     printGolDomain(golDomain, width, height, timestep);
     	// }
	
	  // Send state to neighbor cells
	  for(Vertex &v : cave.hostedVertices){
	      for(auto link : cave.getOutEdges(v)){
		  Vertex destVertex = link.first;
		  Edge   destEdge   = link.second;

		  Vertex& v2 = *(cave.getVertex2(v.id));

		  
		  std::vector<unsigned> send(destEdge.srcIndices.size(),0);

		  auto begin = boost::make_permutation_iterator(v2.core.begin(), destEdge.srcIndices.begin());
		  auto end   = boost::make_permutation_iterator(v2.core.end(), destEdge.srcIndices.end());

		  // std::cout << "(" << v.id << ") ";
		  // for(unsigned u: destEdge.srcIndices)
		  //     std::cout << u << " ";
		  // std::cout << std::endl;
		  
		  std::copy(begin, end, send.begin());
		  
		  events.push_back(cave.asyncSend(destVertex, destEdge, send));
	      }
	  }

     	// Recv state from neighbor cells
     	for(Vertex &v : cave.hostedVertices){
     	    for(auto link : cave.getInEdges(v)){
     		Vertex srcVertex = link.first;
     		Edge   srcEdge   = link.second;
		std::vector<unsigned> recv(srcEdge.srcIndices.size(), 0);
     		cave.recv(srcVertex, srcEdge, recv);

		// std::cout << "(" << v.id << ") ";
		// for(unsigned u: srcEdge.destIndices)
		//     std::cout << u << " ";
		// std::cout << std::endl;
		
		auto begin = boost::make_permutation_iterator(v.border.begin(), srcEdge.destIndices.begin());
		
		std::copy(recv.begin(), recv.end(), begin);
		
		//for(unsigned u: srcVertex.border){
		// for(unsigned u: testV){
		//     std::cout << u << " ";
		// }
		// std::cout << std::endl;
		
     		//if(srcVertex.isAlive[0]) v.aliveNeighbors++;
     	    }
	    // std::cout << "(" << v.coordinates().first << "," << v.coordinates().second << ") ";
	    // for(Cell c: v.border){
	    // 	std::cout << c << " " ;
	    // }
	    // std::cout << std::endl;
     	}

     	// Wait to finish events
     	for(unsigned i = 0; i < events.size(); ++i){
     	    events.back().wait();
     	    events.pop_back();
     	}

	// Calculate state for next generation
	//updateState(cave.hostedVertices);

	//Gather state by vertex with id = 0
	for(Vertex &v: cave.hostedVertices){
	    //v.aliveNeighbors = 0;
	    cave.gather(root, v, v.core, golDomain, true);
      	}

	if(cave.peerHostsVertex(root)){
	    for(auto a: golDomain)
		std::cout << a;
	    std::cout << std::endl;
	}
	
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
