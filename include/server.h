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
  int NodeID;
	int serve(std::string port, std::queue<rpc_msg>& recv_q, std::mutex& recv_mtx);
	void * get_in_addr(struct sockaddr *sa);

};

#endif


/*
  void kdht::handle_function(std::vector<std::string> argvec) {
  std::lock_guard<std::mutex> lck(storage_mtx);
  switch(stoi(argvec[0])) {
    case 0: rcv_q.emplace([=](){
              KVdisplay(argvec);
            });
    default: std::cout << "default\n";

 }
}


while(1){
if ((numbytes = recvfrom(sockfd, (char*)buf.data(), 100, 0,
      (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
    }

    printf("listener: got packet from %s\n",
    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s));

    msgpack::object_handle oh = msgpack::unpack(buf.data(),100);
    msgpack::object obj = oh.get();
    std::cout << obj << std::endl;
    std::vector<std::string> rvec;
    obj.convert(rvec);
    handle_function(rvec);
}

  */
