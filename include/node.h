#ifndef NODE_H_
#define NODE_H_

#include <stdio.h>
#include <stdlib.h>
include <string.h>
include <msgpack.hpp>
#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>

class Node {
public:
	void start(std::string ip, std::string port);
	Node(){
		n_server = 

private:
	
	Server n_server;
	Client n_client;

	std::queue<std::function<void()>> pending_ops_q {};
  std::queue<std::function<void()>> pending_send_q {};
  std::thread client_thread {};
  std::thread server_thread {};
	std::mutex msg_mtx {};


};

#endif 
