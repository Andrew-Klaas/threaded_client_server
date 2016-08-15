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
#include <openssl/sha.h>
#include <limits>
#include <random>

#include "client.h"
#include "server.h"
#include "types.h"

//#define MAXDATASIZE 4096

class Node {
public:
  std::string ip;
  std::string port;
  bool debug;
  int nodeID;
  Node(int);

	void start(std::string ip, std::string port);
  void join();

  void packRpcSendReq(rpc_msg& rpc, std::string fn, 
      std::string ip, std::string port, 
      std::vector<std::string>& args);
  
  std::string ReqPeerID(std::string ip, std::string port, std::size_t length);
      std::string random_string( std::size_t length );
  void sendReqPeerID(std::string ip, std::string port, std::size_t length);
  void sendReplyPeerID(std::string ip, std::string port, std::size_t length);
  void printPeerID(std::string result);
  
  void msg_handler(std::vector<std::string> args);

  void ReqHash(std::string ip, std::string port, std::string hash_fn, const char* data);
  void sendReqHash(std::string ip, std::string port, std::string hash_fn, const char*
      data);

  void hash_handler(std::vector<std::string> args);
  void calcSHA1(std::vector<std::string>& args);

  //void sendReplyPeerID(std::string ip, std::string port, std::size_t length);



  

  std::mutex recv_mtx {};
  std::mutex send_mtx {};

private:
	Server n_server;
	Client n_client;

	std::queue<std::function<void()>> pending_ops_q {};
  std::queue<rpc_msg> pending_send_q {};
  std::queue<std::vector<std::string>> recv_q {};
  std::thread client_thread {};
  std::thread server_thread {};
  std::thread run_thread {};
	std::mutex pending_mtx {};
  bool Running;
};

#endif 

