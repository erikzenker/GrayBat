// Communication
#include <graybat.hpp>

// STL
#include <iostream>   /* std::cout */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <cmath>      /* sqrt */
#include <cstdlib>    /* atoi */
#include <utility>    /* std::pair, std::make_pair */
#include <algorithm>  /* std::transform */

// Boost
#include <boost/iterator/permutation_iterator.hpp> /* boost::make_permutation_iterator*/

#define WIDTH 2

typedef unsigned Cell;


struct SubGrid : public graybat::graphPolicy::SimpleProperty{
    SubGrid() : SubGrid(0) {}
    SubGrid(ID id) : SimpleProperty(id),
		     core(9,1),
		  border(16,0)
    {
	for(unsigned i = 0; i < core.size();++i){
	    unsigned random = rand() % 10000;
	    //std::cout << id << " " << random << std::endl;
	    if(random < 3125){
		core.at(i) = id;
	    }
	}

    }
	
    std::vector<Cell> core;
    std::vector<Cell> border;
    std::pair<unsigned, unsigned> coordinates(){
	unsigned x = id % WIDTH;
	unsigned y = (id / WIDTH);

	return std::make_pair(x,y);
    }
    
};

struct Link : public graybat::graphPolicy::SimpleProperty{
    Link() : Link(0){}
    Link(ID id) : SimpleProperty(id){

    }

    typedef std::vector<Cell>::iterator it;

    std::vector<Cell> srcIndices;
    std::vector<Cell> destIndices;

};




void printGolDomain(const std::vector<unsigned> domain, const unsigned width, const unsigned height, const unsigned subWidth, const unsigned generation){

    unsigned x = 0;
    unsigned y = 0;
    unsigned n = 0;
    
    for(unsigned i = 0; i < domain.size(); ++i){

	unsigned subgridX = subWidth;
	unsigned subgridY = subWidth;
	unsigned gridX    = WIDTH;
	unsigned gridY    = WIDTH;

	if((i % subgridX) == 0 && i != 0)
	    x = (x + 1) % gridX;

	if((i % (subgridX * subgridY * gridX)) == 0 && i != 0)
	    y = (y + 1) % gridY;

	n = (i % subgridX) + (((i / (subgridX * gridX)) % subgridY) * subgridX);
	
	if((i % (width)) == 0){
	    std::cerr << std::endl;
	}

	unsigned j = (x * subgridX * subgridY) + (y * subgridX * subgridY * gridX) + n;
	
	std::cerr << domain.at(j) << " ";
	
	// if(domain.at(j)){
	//   std::cerr << "#";
	// }
	// else {
	//     std::cerr << " ";
	// }


	

    }
    std::cerr << "Generation: " << generation << std::endl;
    for(unsigned i = 0; i < height+1; ++i){
	std::cerr << "\033[F";
    }

}


template <class T>
void updateState(std::vector<T> &cells){
    for(T &cell : cells){
	updateState(cell);

    }

}


template <class T>
void updateState(T &cell){
    std::array<std::vector<unsigned>, 9> coreNeighbors;
    std::array<std::vector<unsigned>, 9> borderNeighbors;

    borderNeighbors[0] = std::vector<unsigned>{{15,14,13,0,2}};
    borderNeighbors[1] = std::vector<unsigned>{{0,1,2}};
    borderNeighbors[2] = std::vector<unsigned>{{1,2,3,4,5}};
    borderNeighbors[3] = std::vector<unsigned>{{14,13,12}};
    borderNeighbors[4] = std::vector<unsigned>{{}};
    borderNeighbors[5] = std::vector<unsigned>{{4,5,6}};
    borderNeighbors[6] = std::vector<unsigned>{{13,12,11,10,9}};
    borderNeighbors[7] = std::vector<unsigned>{{8,9,10}};
    borderNeighbors[8] = std::vector<unsigned>{{5,6,7,8,9}};

    coreNeighbors[0] = std::vector<unsigned>{{1,3,4}};
    coreNeighbors[1] = std::vector<unsigned>{{0,2,3,4,5}};
    coreNeighbors[2] = std::vector<unsigned>{{1,4,5}};
    coreNeighbors[3] = std::vector<unsigned>{{0,1,4,6,7}};
    coreNeighbors[4] = std::vector<unsigned>{{0,1,2,3,5,6,7,8}};
    coreNeighbors[5] = std::vector<unsigned>{{1,2,4,7,8}};
    coreNeighbors[6] = std::vector<unsigned>{{3,4,7}};
    coreNeighbors[7] = std::vector<unsigned>{{6,3,4,5,8}};
    coreNeighbors[8] = std::vector<unsigned>{{7,4,5}};
    

 
    for(unsigned i = 0; i < cell.core.size(); i++){
    	unsigned nNeighbors = 0;
    	for(auto c: coreNeighbors[i]){
    	    if(cell.core[c] > 0){
    		nNeighbors++;
    	    }
    	}

    	for(auto b: borderNeighbors[i]){
    	    if(cell.border[b] > 0){
    		nNeighbors++;
    	    }
    	}

    	//cell.core[i] = (cell.core[i] + 1 ) % 2;

	// switch(nNeighbors){

    	// case 0:
    	// case 1:
    	//     cell.core[i] = 0;
    	//     break;

    	// case 2:
    	//     cell.core[i] = cell.core[i];
    	//     break;
	    
    	// case 3: 
    	//     cell.core[i] = 1;
    	//     break;

    	// default: 
    	//     cell.core[i] = 0;
    	//     break;

    	// };

    }

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
    typedef graybat::graphPolicy::BGL<SubGrid, Link> GP;
    
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

    const unsigned height = WIDTH;
    const unsigned width  = WIDTH;

    // Init random numbers
    srand(1234567);
    
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

     	// Print life field by owner of vertex 0
     	if(cave.peerHostsVertex(root)){
     	    printGolDomain(golDomain, width*3, height*3, 3, timestep);
     	}
	
	  // Send state to neighbor cells
	  for(Vertex &v : cave.hostedVertices){
	      for(auto link : cave.getOutEdges(v)){
		  Vertex destVertex = link.first;
		  Edge   destEdge   = link.second;

		  //Vertex& v2 = *(cave.getVertex2(v.id));

		  std::vector<unsigned> send(destEdge.srcIndices.size(),0);

		  auto begin = boost::make_permutation_iterator(v.core.begin(), destEdge.srcIndices.begin());
		  auto end   = boost::make_permutation_iterator(v.core.end(), destEdge.srcIndices.end());

		  // std::cout << "(" << v.id << ") ";
		  // for(unsigned u: destEdge.srcIndices)
		  //     std::cout << u << " ";
		  // std::cout << std::endl;
		  
		  //std::copy(begin, end, send.begin());
		  
		  //events.push_back(cave.asyncSend(destVertex, destEdge, send));
	      }
	  }

     	// Recv state from neighbor cells
     	for(Vertex &v : cave.hostedVertices){
     	    for(auto link : cave.getInEdges(v)){
     		Vertex srcVertex = link.first;
     		Edge   srcEdge   = link.second;
		std::vector<unsigned> recv(srcEdge.srcIndices.size(), 0);
     		//cave.recv(srcVertex, srcEdge, recv);

		// std::cout << "(" << v.id << ") ";
		// for(unsigned u: srcEdge.destIndices)
		//     std::cout << u << " ";
		// std::cout << std::endl;
		
		auto begin = boost::make_permutation_iterator(v.border.begin(), srcEdge.destIndices.begin());
		
		//std::copy(recv.begin(), recv.end(), begin);
		
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
	updateState(cave.hostedVertices);

	//Gather state by vertex with id = 0
	for(Vertex &v: cave.hostedVertices){
	    //v.aliveNeighbors = 0;
	    //std::for_each(v.core.begin(), v.core.end(), [](unsigned& a){a = (a + 1) % 2;});
	    //std::for_each(golDomain.begin(), golDomain.end(), [](unsigned& a){a = 0;});
	    cave.gather(root, v, v.core, golDomain, true);
      	}

	 // if(cave.peerHostsVertex(root)){
	 //     for(auto a: golDomain)
	 // 	std::cout << a;
	 //     std::cout << std::endl;
	 // }
	
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
