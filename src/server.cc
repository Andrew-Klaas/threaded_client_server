#include "server.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

Server::Server(int NodeID) : nodeID(NodeID) {};

int sendMsg(int fd, char * buf, int nbytes, int blah);


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void * Server::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int Server::serve(std::string port,
    std::queue<std::vector<std::string>>& recv_q, 
    std::mutex& recv_mtx) {

    printf("Node %d, Server Starting on %s  \n", nodeID, port.c_str()); //TODO change to nodeid
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    //char buf[256];    // buffer for client data
    msgpack::sbuffer buf;
    int nbytes;
    size_t recv_len = 0;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop

    char buffer[1];
    //printf("server: waiting for connections\n");
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        /*
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                            */
                    }
                } else {
                    // handle data from a client
                    // fix 4096
                    if ((nbytes = recv(i, buffer,
                            4, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            //printf("selectserver: socket %d hung up, no bytes\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {

                        printf("Node %d, receiving data\n", nodeID);  

                        unsigned long bytes_left = 0;

                        for ( int i = 3; i >= 0 ; i--){
                          //printf("%x, \n", buffer[i]);
                          bytes_left = bytes_left |  ((buffer[i] & 0xFF)<<8*i);

                        }

                        std::cout << "bytes_left: " << bytes_left << "\n";

                        if (bytes_left > 0) {
                           nbytes = recv(i, (char*)buf.data(),
                               bytes_left, 0);
                        }

                        printf("before unpack\n");
                        std::string s;
                        std::cout << "max string size is " << s.max_size() <<
                          "\n";
                        msgpack::object_handle oh = msgpack::unpack(buf.data(),
                            bytes_left);
                        msgpack::object obj = oh.get();
                        std::vector<std::string> rvec;
                        obj.convert(rvec);
                        printf("after unpack\n");


                        
                        /*
                        printf("recived args begin: \n");
                        for (auto& i : rvec) {
                          std::cout << i << std::endl;
                        }
                        printf("received args end:\n");
                        */
                        

                        std::lock_guard<std::mutex> lck(recv_mtx);
                        recv_q.emplace(rvec);

                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}

int sendMsg(int fd, char * buf, int nbytes, int blah){
  if (send(fd, buf, nbytes, blah) == -1) {
    perror("send");
  }
  return  0;
}



