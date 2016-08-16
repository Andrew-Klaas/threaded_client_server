#include "node.h"

Node::Node(int NodeID) : nodeID(NodeID), n_server(NodeID), n_client(NodeID) {};

void Node::start(std::string ip, std::string port) {
   //unsigned int nthreads = std::thread::hardware_concurrency();
 	if((validateNum(port)) == -1) {
		printf("Node %d, exiting\n", nodeID);
		return;
	} else {
		this->ip = ip;
	}
 
 	//TODO check for valid address, or set to 127.0.0.1
	this->port = port;
 
	Running = true;

  // start server thread
  server_thread = std::thread([&] { 
      this->n_server.serve(port, this->recv_q, recv_mtx); 
  });

  // start client thread
  client_thread = std::thread([&] { 
      this->n_client.serve(ip, port, this->pending_send_q, send_mtx); 
  });
	
	// start thread to watch for pending operations
  run_thread  = std::thread([&] { 
    while(Running){

      decltype(pending_ops_q) pending_ops {};
      { 
        std::lock_guard<std::mutex> lck(pending_mtx);
        pending_ops = std::move(pending_ops_q);
      }
      while (!pending_ops.empty()) {
        pending_ops.front()(); 
        pending_ops.pop();
      }
   }
  });

 	// start thread to watch for pending receives to execute 
  run_thread2  = std::thread([&] { 
     while(Running) {

      decltype(recv_q) recv_ops {};
      {
          std::lock_guard<std::mutex> lck(recv_mtx);
          recv_ops = std::move(recv_q);
      }
      while (!recv_ops.empty()) {
          msg_handler(recv_ops.front());
          recv_ops.pop();
      }
    }
  });

}

void Node::msg_handler(std::vector<std::string> args){
  std::lock_guard<std::mutex> lck(pending_mtx); 
	// check fn and port
	// TODO check address
	if(((validateNum(args[2])) == -1) || (validateNum(args[1])) == -1) {
		printf("Node %d, received Invalid msg \n", nodeID);
		return;
	}
  int result = stoi(args[2]);
   switch(result) {
     case 0:
       break;
     case 1:
       pending_ops_q.emplace([=](){
					if((validateNum(args[3])) == -1) {
						printf("Node %d, received Invalid msg \n", nodeID);
						return;
					}
				 //use std::move()?
         sendReplyPeerID(args[0],args[1],stoi(args[3]));
       });
       //cv.notify_all();
       break;
     case 2:
       pending_ops_q.emplace([=](){
        printPeerID(std::move(args[5]));
       });
       //cv.notify_all();
       break;
     case 3:
       pending_ops_q.emplace([=](){
         hash_handler(std::move(args));
       });
       break;
     case 4:
			 if((validateNum(args[4])) == -1) {
				 printf("Node %d, received Invalid msg \n", nodeID);
  			 return;
			 }
       pending_ops_q.emplace([=](){
        printHash(std::move(args[3]),std::move(args[4]), std::move(args[5]));
       });
     default: break;
   }

}

void Node::hash_handler(std::vector<std::string> args){
  if (args[3] == "SHA1") {
    unsigned char hash_ptr[SHA_1_LENGTH];

    calcSHA1(args, hash_ptr);
		std::string hash_return_size = std::to_string(SHA_1_LENGTH);
		prepareHashRpcSend(args, hash_ptr, hash_return_size);
  } 
  else if (args[3] == "SHA256") {
    unsigned char hash_ptr[SHA_256_LENGTH];
    calcSHA256(args, hash_ptr);
		std::string hash_return_size = std::to_string(SHA_256_LENGTH);
		prepareHashRpcSend(args, hash_ptr, hash_return_size);
  }
	else if (args[3] == "MD5") {
		unsigned char hash_ptr[MD5_LENGTH];
		calcMD5(args, hash_ptr);
		std::string hash_return_size = std::to_string(MD5_LENGTH);
		prepareHashRpcSend(args, hash_ptr, hash_return_size);
	}
  else {
    printf("undefined hash function\n");
		return;
  }
	printf("Node %d, received %s request from peer ... calculating hash\n",nodeID, args[3].c_str());
}

void Node::prepareHashRpcSend(std::vector<std::string>& rpc_args, unsigned char* hash_ptr, std::string hash_return_size){
	rpc_msg rpc;
	rpc.fn = rpc_args[3] + " Response";
	auto ip = rpc_args[0];
	auto port = rpc_args[1];
	std::string result((char*)hash_ptr);
	std::vector<std::string> args = { rpc_args[3], hash_return_size,
      result};
  packRpcSendReq(rpc, "4", ip, port, args);
	pending_send_q.emplace(std::move(rpc));
}

void Node::calcSHA1(std::vector<std::string>& args, unsigned char*
    hash_ptr) {
  auto data_ptr = args[5].c_str();

  SHA1((unsigned char*)data_ptr, args[5].size(), hash_ptr);
}

void Node::calcSHA256(std::vector<std::string>& args, unsigned char*
    hash_ptr) {
  auto data_ptr = args[5].c_str();

  SHA256((unsigned char*)data_ptr, args[5].size(), hash_ptr);
}

void Node::calcMD5(std::vector<std::string>& args, unsigned char*
    hash_ptr) {
  auto data_ptr = args[5].c_str();

  MD5((unsigned char*)data_ptr, args[5].size(), hash_ptr);
}

void Node::printPeerID(std::string result) {
	printf("Node %d, ID received: %s\n",nodeID, result.c_str());

}

std::string Node::ReqPeerID(std::string ip, std::string port, std::size_t length){
  std::lock_guard<std::mutex> lck(pending_mtx); 
  pending_ops_q.emplace([=](){
      sendReqPeerID(ip, port, length);
  });
  //cv.notify_all();
  return "TODO";
}

void Node::printHash(std::string hash_fn, std::string length, std::string result) {
  auto char_result = result.c_str();
  printf("Node %d, received %s hash: ",nodeID, hash_fn.c_str());
  for(auto i = 0 ; i  < stoi(length); i++) {
      printf("%x",(unsigned char)char_result[i]);
  }
  printf("\n");
}

void Node::sendReqPeerID(std::string ip, std::string port, std::size_t length){
	printf("Node %d, RequestID from  %s:%s\n", nodeID, ip.c_str(), port.c_str());
  rpc_msg rpc;
	rpc.fn = "ID Request";
  auto length_s = std::to_string(length);
  std::vector<std::string> args = { length_s, "0", "" };
  packRpcSendReq(rpc, "1", ip, port, args);
  pending_send_q.emplace(std::move(rpc));
}

void Node::sendReplyPeerID(std::string ip, std::string port, std::size_t length){
	printf("Node %d, received request for ID\n", nodeID);
  rpc_msg rpc;
  rpc.fn = "ID Response";	
  auto result = random_string(length);
  std::vector<std::string> args = { "", std::to_string(result.length()), result };
  packRpcSendReq(rpc, "2", ip, port, args);
  pending_send_q.emplace(std::move(rpc));
}

std::string Node::random_string( std::size_t length ) {
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar );
    return str;
}


void Node::packRpcSendReq(rpc_msg& rpc, std::string fn, std::string ip, std::string port, std::vector<std::string>& args){ 
  rpc.ip = ip;
  rpc.port = port;
  rpc.vec.push_back(this->ip);    
  rpc.vec.push_back(this->port);    
  rpc.vec.push_back(fn); 
  rpc.vec.push_back(args[0]);  // length or hash function
  rpc.vec.push_back(args[1]);  // size
  rpc.vec.push_back(args[2]);  // data
  msgpack::pack(rpc.buffer, rpc.vec);

}

void Node::ReqHash(std::string ip, std::string port, std::string hash_fn, const char* data){
  std::lock_guard<std::mutex> lck(pending_mtx); 
  pending_ops_q.emplace([=](){
      sendReqHash(ip, port, hash_fn, data);
  });
  //cv.notify_all();
}

void Node::sendReqHash(std::string ip, std::string port, std::string hash_fn,
    const char* data){
	printf("Node %d, Requesting %s Hash \n", nodeID, hash_fn.c_str());
  rpc_msg rpc;
	rpc.fn = hash_fn + " Request";
  std::string data_s(data);
  std::vector<std::string> args = { hash_fn, std::to_string(data_s.length()) , data_s };
  packRpcSendReq(rpc, "3", ip, port, args);
  pending_send_q.emplace(std::move(rpc));

}


void Node::join() {
  Running = false;
  run_thread.join();
  run_thread2.join();
  server_thread.join();
  client_thread.join();
}

int Node::validateNum(std::string port){
	int x;
	try {
		x = stoi(port);
	}
	catch(std::invalid_argument& e){
		printf("Invalid Port Number!\n");
		return -1;
	}
	return 0;
}

