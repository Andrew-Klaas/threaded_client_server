#ifndef NODE_H_
#define NODE_H_

#include <stdexcept>
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
#include <openssl/md5.h>
#include <limits>
#include <random>

#include "client.h"
#include "server.h"
#include "types.h"

#define SHA_1_LENGTH 20
#define SHA_256_LENGTH 32 
#define MD5_LENGTH 16

class Node {
public:
  std::string ip;
  std::string port;
  bool debug;
  int nodeID;
  
  Node(int); //int is node's startup ID
	int start(std::string ip, std::string port);  //Start threads
  void join(); //Stop Node
  
  /* Request Peer ID
   * ip: IP to connect to
   * port: port to connect to
   * length: length of random ID peer is requesting
   */
  void ReqPeerID(std::string ip, std::string port, std::size_t length);
  
  /* Request Hash
   * ip: IP to connect to
   * port: port to connect to
   * hash_fn: hash function to calculate -> "SHA1", "SHA25", "MD5"
   * data: data block to hash
   */
  void ReqHash(std::string ip, std::string port, std::string hash_fn, const char* data);

private:

  //delegate remote procedural calls
  void msg_handler(std::vector<std::string> args);

  int validateNum(std::string port);
	bool validateIpAddress(const std::string &ipAddress);

  //PeerID functions
  std::string random_string( std::size_t length ); 
  void sendReqPeerID(std::string ip, std::string port, std::size_t length);
  void sendReplyPeerID(std::string ip, std::string port, std::size_t length);
  //show received random ID
  void printPeerID(std::string result);

  //Hash related Functions
  void sendReqHash(std::string ip, std::string port, std::string hash_fn, const char*
      data);
  void prepareHashRpcSend(std::vector<std::string>& args,unsigned char* hash_ptr,
      std::string hash_return_size);
  // Chooses Hash function wrapper
  void hash_handler(std::vector<std::string> args);
  // show received hash result
  void printHash(std::string hash_fn, std::string length, std::string result);

  //Actual hash function wrappers
  void calcSHA1(std::vector<std::string>& args, unsigned char* hash_ptr);
  void calcSHA256(std::vector<std::string>& args, unsigned char* hash_ptr);
	void calcMD5(std::vector<std::string>& args, unsigned char* hash_ptr);

  // pack the RPC message via messagepack
  void packRpcSendReq(rpc_msg& rpc, std::string fn, 
      std::string ip, std::string port, 
      std::vector<std::string>& args);

//private:
  // inbound messages
	Server n_server;
  // outbound messages
	Client n_client;

  std::mutex recv_mtx {};
  std::mutex send_mtx {};
	std::mutex pending_mtx {};

	std::queue<std::function<void()>> pending_ops_q {};
  std::queue<rpc_msg> pending_send_q {};
  std::queue<std::vector<std::string>> recv_q {};
  
	std::thread client_thread {};
  std::thread server_thread {};
  std::thread pending_ops_thread {};
  std::thread recv_ops_thread {};

  bool Running;
};

#endif 

