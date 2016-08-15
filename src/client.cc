/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.h"

//#define PORT "3490" // the port client will be connecting to 
Client::Client(int NodeID) : nodeID(NodeID) {};

// get sockaddr, IPv4 or IPv6:
void * Client::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int Client::serve(std::string ip, std::string port, 
    std::queue<rpc_msg>& pending_send_q, 
    std::mutex& send_mtx) {

    Running = true;
    //printf("client running\n");
    while (Running) {
      if (!pending_send_q.empty()) {

        int sockfd, numbytes, bytes_sent;
        int rv;
        struct addrinfo hints, *servinfo, *p;
        char s[INET6_ADDRSTRLEN];
        
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        std::lock_guard<std::mutex> lck(send_mtx);
        auto rpc = std::move(pending_send_q.front());
        //std::cout << "buffer: " << rpc.buffer.data() << std::endl;
        pending_send_q.pop();
        
        auto ip = std::move(rpc.ip);
        auto port = std::move(rpc.port);
        auto buffer = std::move(rpc.buffer);
        std::size_t len;
        len = buffer.size();
        
        if ((rv = getaddrinfo(ip.c_str() ,port.c_str(), &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and connect to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("client: connect");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof s);
        
        freeaddrinfo(servinfo); // all done with this structure


        //printf("rpc.buffer.size %zu \n",buffer.size());

        if (sendall(sockfd, (char*)buffer.data(), &len) == -1) {
               perror("sendall");
        }
        close(sockfd);

      } // if statement
    } // while loop

    return 0;
}

int Client::sendall( int s, char *buf, unsigned long *len){
  unsigned long n = 0, total = 0;
  
  auto bytesleft = *len;
 
  unsigned char * p = (unsigned char*)&(*len);

  /*
  printf("\n");
  for (int i = 0; i < 4; i++){
    printf( "%x, i: %d\n", p[i], i);
  }
  printf("\n");
  */
  
  auto first = send(s, p,4,0);

  while(total < *len) {
    n = send(s, buf+total, bytesleft, 0 );
    if (n == -1) {break;}
    total += n;
    bytesleft -= n;
  }

  *len = total;

  return n == -1 ? -1 : 0;
  return 0;
}

