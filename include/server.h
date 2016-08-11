#ifndef SERVER_H_
#define SERVER_H_

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


class Server {
public: 
  int NodeID;
	int serve(std::string port, std::queue<std::function<void()>> pending_ops_q);
	void * get_in_addr(struct sockaddr *sa);

};
#endif

