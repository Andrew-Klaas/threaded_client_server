#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
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
#include <functional>
#include <queue>
#include <string>
#include <vector>
#include <mutex>

#include "types.h"

class Client {
public:
  Client(int);
  int nodeID;
	void *get_in_addr(struct sockaddr *sa);
	int serve(std::string ip, std::string port, 
      std::queue<rpc_msg>& pending_send_q, 
      std::mutex& send_mtx);
  int sendall( int s, char *buf, unsigned long *len);
  bool Running;

};

#endif
