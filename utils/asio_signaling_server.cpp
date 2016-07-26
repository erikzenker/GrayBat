
// CLIB
#include <string.h>   /* strup */

// STL
#include <iostream> /* std::cout, std::endl */
#include <map>      /* std::map */
#include <string>   /* std::string */
#include <sstream>  /* std::stringstream */

// Boost.Asio
#include <boost/asio.hpp>

// Boost
#include <boost/program_options.hpp>

#define CHUNK_SIZE 8096

// Type defs
typedef unsigned Tag;                                            
typedef unsigned ContextID;
typedef unsigned VAddr;
typedef unsigned MsgType;
typedef std::string Uri;

// Message tags
static const MsgType VADDR_REQUEST   = 0;
static const MsgType VADDR_LOOKUP    = 1;
static const MsgType DESTRUCT        = 2;
static const MsgType RETRY           = 3;
static const MsgType ACK             = 4;
static const MsgType CONTEXT_INIT    = 5;
static const MsgType CONTEXT_REQUEST = 6;

template <typename T_Socket>            
void recvFromSocket(T_Socket& socket, std::stringstream& ss) {
    const size_t size = ss.str().size();
    std::array<char, CHUNK_SIZE> chunk; 
    size_t transferred = 0;

    while (transferred != size) { 
        size_t remaining = size - transferred; 
        size_t read_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
        boost::asio::read(socket, boost::asio::buffer(chunk, read_size)); 
        ss.write(chunk.data(), read_size); 
        transferred += read_size; 
    }
                
}

template <typename T_Socket>
void sendToSocket(T_Socket& socket, std::stringstream & ss) {
    const size_t size = ss.str().size();
    size_t transferred = 0;
    std::array<char, CHUNK_SIZE> chunk;
                
    while (transferred != size){ 
        size_t remaining = size - transferred; 
        size_t write_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
        ss.read(chunk.data(), CHUNK_SIZE); 
        boost::asio::write(socket, boost::asio::buffer(chunk, write_size)); 
        transferred += write_size; 
    } 

}

void recvHandler(const boost::system::error_code& error,std::size_t bytes_transferred){

}

void acceptHandler(const boost::system::error_code& error){

    std::cout << "Accept" << std::endl;
    
    if(!error){

    }

    
}



int main(const int argc, char **argv){
    /***************************************************************************
     * Parse Commandline
     **************************************************************************/
    namespace po = boost::program_options;
    po::options_description options( "ZMQ Signaling Server Options" );
    
    options.add_options()
        ("port,p",
         po::value<unsigned>()->default_value(6000),
         "Port to listen for signaling requests")
        ("ip",
         po::value<std::string>(),
         "IP to listen for signaling requests. Either ip or interface can be specified. (Example: 127.0.0.1)")
		("interface,i",
         po::value<std::string>(),
         "Interface to listen for signaling requests. Either ip or interface can be specified. Default are all available interfaces. (Example: eth0)")
		("protocoll",
         po::value<std::string>()->default_value("tcp"),
         "Protocoll to listen for signaling requests. Options are tcp and udp. Default is udp.")
        ("help,h",
         "Print this help message and exit");


    po::variables_map vm;
    po::store(po::parse_command_line( argc, argv, options ), vm);
    
    if(vm.count("help")){
        std::cout << "Usage: " << argv[0] << " [options] " << std::endl;
        std::cout << options << std::endl;
        exit(0);
    }
    
    if(vm.count("ip") && vm.count("interface")) {
		std::cerr << "Error: Only one of the following can be specified by parameter. Either ip or interface." << std::endl;
		exit(1);
	}
	
	std::string masterUri;
	if(vm.count("ip")) {
		//Listen to ip
		masterUri = vm["protocoll"].as<std::string>() + "://" + vm["ip"].as<std::string>() + ":" + std::to_string(vm["port"].as<unsigned>());
	} else  if(vm.count("interface")) {
		//Listen to interface
		masterUri = vm["protocoll"].as<std::string>() + "://" + vm["interface"].as<std::string>() + ":" + std::to_string(vm["port"].as<unsigned>());	
	} else {
		//Listen to all interfaces
		masterUri = vm["protocoll"].as<std::string>() + "://*:" + std::to_string(vm["port"].as<unsigned>());
	}
	
    /***************************************************************************
     * Start signaling
     **************************************************************************/
    
    std::cout << "Start asio signaling server" << std::endl;

    std::map<ContextID, std::map<VAddr, Uri> > phoneBook;
    std::map<ContextID, VAddr> maxVAddr;

    std::cout << "Listening on: " << masterUri << std::endl;
    
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), vm["port"].as<unsigned>());    
    boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint); 

    boost::asio::ip::tcp::socket socket(io_service);
    acceptor.async_accept(socket, acceptHandler);
    //acceptor.accept(socket);

    //std::cout << "accepted connection" << std::endl;

    io_service.run();
    
    while(true){
    }

    
    // ContextID maxContextID = 0;
    // ContextID maxInitialContextID = maxContextID;    

    // while(true){
    //     std::stringstream recvStream;
    //     std::stringstream sendStream;        




        


    //     recvFromSocket(socket, recvStream);

    //     Uri srcUri;
    //     MsgType type;
    //     ContextID contextID;
    //     unsigned size;
    //     recvStream >> type;

    //     switch(type){

    //     case CONTEXT_INIT:
    //         {

    //     	recvStream >> size;
    //     	if(maxVAddr[maxInitialContextID]  == size){
    //     	    maxInitialContextID = ++maxContextID;
    //     	    maxVAddr[maxInitialContextID] = 0;
    //     	}

    //             sendStream << std::to_string(maxInitialContextID) << " ";
    //     	sendToSocket(socket, sendStream);
    //     	std::cout << "CONTEXT INIT [size:" << size << "]: " << maxInitialContextID << std::endl;		
    //     	break;
    //         }

    //     case CONTEXT_REQUEST:
    //         {
    //     	recvStream >> size;
    //     	maxVAddr[++maxContextID] = 0;

    //             sendStream << std::to_string(maxContextID) << " ";
    //     	sendToSocket(socket, sendStream);
    //     	std::cout << "CONTEXT REQUEST [size:" << size << "]: " << maxContextID << std::endl;				
    //     	break;

    //         }
	    
    //     case VADDR_REQUEST:
    //         {
    //             // Reply with correct information
    //     	recvStream >> contextID;
    //             recvStream >> srcUri;

    //             phoneBook[contextID][maxVAddr[contextID]] = srcUri;

    //             sendStream << std::to_string(maxVAddr[contextID]) << " ";
    //             sendToSocket(socket, sendStream);
    //     	std::cout << "VADDR REQUEST [contextID:" << contextID << "][srcUri:" << srcUri << "]: " << maxVAddr[contextID] << std::endl;
    //     	maxVAddr[contextID]++;		
    //             break;
    //         }
                        
    //     case VADDR_LOOKUP:
    //         {
    //             VAddr remoteVAddr;
    //     	recvStream >> contextID;
    //             recvStream >> remoteVAddr;		
		

    //             if(phoneBook[contextID].count(remoteVAddr) == 0){
    //                 sendStream << RETRY << " ";
    //                 sendToSocket(socket, sendStream);
    //     	    std::cout << "VADDR LOOKUP [contextID:" << contextID << "][remoteVAddr:" << remoteVAddr << "]: " << " RETRY"<< std::endl;		    		    
    //             }
    //             else {
    //                 sendStream << ACK << " " << phoneBook[contextID][remoteVAddr] << " ";
    //                 sendToSocket(socket, sendStream);
    //     	    std::cout << "VADDR LOOKUP [contextID:" << contextID << "][remoteVAddr:" << remoteVAddr << "]: " << phoneBook[contextID][remoteVAddr] << std::endl;
    //             }

    //             break;
    //         }

    //     case DESTRUCT:
    //         sendStream << " ";
    //         sendToSocket(socket, sendStream);
    //         std::cout << "DESTRUCT" << std::endl;
    //         break;
                        
    //     default:
    //         // Reply empty message
    //         sendStream << " ";
    //         sendToSocket(socket, sendStream);
    //         std::cout << "UNKNOWN MESSAGE TYPE" << std::endl;
    //         exit(0);
    //         break;

    //     };
                    
    // }

}
