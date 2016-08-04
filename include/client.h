#ifndef CLIENT_H_
#define CLIENT_H_

//common.h ?
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

class Client {
public:
	void *get_in_addr(struct sockaddr *sa);
	int send(int argc, char *argv[]);




};

#endif
