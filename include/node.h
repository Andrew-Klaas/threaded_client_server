#ifndef NODE_H_
#define NODE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.hpp>
#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <queue>
#include <random>
#include <functional> //for std::function
#include <algorithm>  //for std::generate_n
#include <string>

#include "client.h"
#include "server.h"
#include "types.h"

#define MAXDATASIZE 4096

class Node {
public:
  int nodeID;

	void start(std::string ip, std::string port);
  //unsigned char *
  std::string ReqPeerID(std::string ip, std::string port, std::size_t length);
  //unsigned char *
  //unsigned char *ReqHash(std::string ip, std::string port, std::string fn, const unsigned char & data);
  void sendReqPeerID(std::string ip, std::string port, std::size_t length);
  std::string random_string( std::size_t length );
  //void prepareSockRpcSend(rpc_msg& rpc, std::string ip, std::string port);
  void packRpcSendReq(rpc_msg& rpc, std::string fn, std::string ip, std::string port);
  void join();

  std::mutex recv_mtx {};
  std::mutex send_mtx {};

private:
	
	Server n_server;
	Client n_client;

	std::queue<std::function<void()>> pending_ops_q {};
  std::queue<rpc_msg> pending_send_q {};
  std::queue<rpc_msg> recv_q {};
  std::thread client_thread {};
  std::thread server_thread {};
  std::thread run_thread {};
	std::mutex pending_mtx {};
  
  bool Running;


  /*
     Blob
     std::vector<unsigned char>
  */

};


#endif 

