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
    printf("client running\n");
    while (Running) {
      if (!pending_send_q.empty()) {

        int sockfd, numbytes, bytes_sent;
        int rv;
        struct addrinfo hints, *servinfo, *p;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        char s[INET6_ADDRSTRLEN];

        std::lock_guard<std::mutex> lck(send_mtx);
        auto rpc = std::move(pending_send_q.front());
        pending_send_q.pop();
        
        //auto sockfd = std::move(rpc.sockfd);
        //auto numbytes = std::move(rpc.numbytes);
        //auto rv = std::move(rpc.rv);
        auto ip = std::move(rpc.ip);
        auto port = std::move(rpc.port);
        //auto hints = std::move(rpc.hints);
        //auto servinfo = std::move(rpc.servinfo);
        //auto p = std::move(rpc.p);
        auto buffer = std::move(rpc.buffer);
        //*/
       
        if ((rv = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
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
        
        printf("client: connecting to %s\n", s);

        freeaddrinfo(servinfo); // all done with this structure
        
        if (send(sockfd, (char*)buffer.data(), buffer.size(), 0) == -1) {
               perror("send");
        }
        close(sockfd);
      } // if statement
    } // while loop

    return 0;
}


