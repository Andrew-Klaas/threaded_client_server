#ifndef TYPES_H_
#define TYPES_H_

#include <msgpack.hpp>
#include <vector>

struct rpc_msg { 
    std::string ip;
    std::string port;
    // change this to something else, not a string
    std::string fn;

    std::size_t hash_length;

    //sock info
    int sockfd, numbytes;
    //char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    //std::vector<unsigned char> blob
    std::vector<std::string> vec;
    msgpack::sbuffer buffer;
  
};


#endif
