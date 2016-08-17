#ifndef SERVER_H_
#define SERVER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <msgpack.hpp>
#include <queue>
#include <mutex>

#include "types.h"

class Server {
public: 
  int nodeID;
  Server(int);
	int serve(std::string port, std::queue<std::vector<std::string>>& recv_q, std::mutex& recv_mtx);
	void * get_in_addr(struct sockaddr *sa);

};

#endif

