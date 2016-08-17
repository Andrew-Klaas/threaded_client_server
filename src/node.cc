#include "node.h"

/*
 *  RPC protocol layout
 *  The RPC messages are implemented using msgpackC and std::vector<std::string>
 *  example with a -> std::vector<std::string> msg
 *  msg[0] = Source IP
 *  msg[1] = Source Port
 *  msg[2] = Function to execute-> 
 *     0: Undefined, 1: Request PeerID, 2: PeerID response, 3: Hash request, 4: Hash response
 *  msg[3] = Length of requested PeerID, or the type of hash requested
 *  msg[4] = Size of requested PeerID in bytes, or blob to hash in bytes
 *  msg[5] = The data, example: the random PeerID or the hash value of the blob
 */

Node::Node(int NodeID) : nodeID(NodeID), n_server(NodeID), n_client(NodeID) {};

/* Start Server
 *  This function starts the client and server threads
 * as well to watch the Pending_ops_q and the  recv_q.
 * 
 * When the Peer RPC's are called, their functions are 
 * placed on the pending_ops_q. Then the run_thead will pull
 * from the queue and execute before placing on teh pending_send_q
 * for the client to send out on the wire.
 *
 * when Peer RPC's are received, their args are placed in the recv_q
 * by the server. The args are pulled from the q and sent to the msg_handler
 * by the run_thread2.
 * 
 */
int Node::start(std::string ip, std::string port) {
   //unsigned int nthreads = std::thread::hardware_concurrency();
 	if((validateNum(port)) == -1) {
		printf("Node %d, exiting\n", nodeID);
		return -1;
	} else {
		this->port = port;
	}
	if(!validateIpAddress(ip)){
		printf("Node %d, invalid IP, exiting\n", nodeID);
		return -1;
  } else { 
		this->ip = ip;
	}
 
	Running = true; 

  // start server thread
  server_thread = std::thread([&] { 
      this->n_server.serve(port, this->recv_q, recv_mtx); 
  });

	//Only calling this to let the server start up
	sleep(1);

  // start client thread
  client_thread = std::thread([&] { 
      this->n_client.serve(ip, port, this->pending_send_q, send_mtx); 
  });
	
	// start thread to watch for pending operations to execute
  pending_ops_thread  = std::thread([&] { 
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

 	// start thread to watch for pending received operations from server
  recv_ops_thread  = std::thread([&] { 
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
	return 0;
}

// Handle incoming messages from server
void Node::msg_handler(std::vector<std::string> args){
  std::lock_guard<std::mutex> lck(pending_mtx); 
	
	if(((validateNum(args[2])) == -1) || (validateNum(args[1])) == -1) {
		printf("Node %d, received Invalid msg \n", nodeID);
		return;
	}
	if(!validateIpAddress(args[0])){
		printf("Node %d, received invalid IP\n", nodeID);
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

/* Hash related functions
 *
 *
 */
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
	printf("Node %d, received %s request of %lu bytes of data from peer ... calculating hash\n", nodeID,args[3].c_str(),args[5].size() );
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

void Node::printHash(std::string hash_fn, std::string length, std::string result) {
  auto char_result = result.c_str();
  printf("Node %d, received %s hash: ",nodeID, hash_fn.c_str());
  for(auto i = 0 ; i  < stoi(length); i++) {
      printf("%x",(unsigned char)char_result[i]);
  }
  printf("\n");
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
	printf("Node %d, Requesting %s Hash of %lu bytes \n", nodeID, hash_fn.c_str(),
      strlen(data));
  rpc_msg rpc;
	rpc.fn = hash_fn + " Request";
  std::string data_s(data);
  std::vector<std::string> args = { hash_fn, std::to_string(data_s.length()) , data_s };
  packRpcSendReq(rpc, "3", ip, port, args);
  pending_send_q.emplace(std::move(rpc));

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

/* PeerID functions
 *
 *
 */
void Node::ReqPeerID(std::string ip, std::string port, std::size_t length){
  std::lock_guard<std::mutex> lck(pending_mtx); 
  pending_ops_q.emplace([=](){
      sendReqPeerID(ip, port, length);
  });
  //cv.notify_all();
  return;
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

void Node::printPeerID(std::string result) {
	printf("Node %d, ID received: %s\n",nodeID, result.c_str());

}

void Node::join() {
	printf("Node %d, Shutting down\n", nodeID);
  Running = false;
	n_server.Running = false;
	n_client.Running = false;
  pending_ops_thread.join();
  recv_ops_thread.join();
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
bool Node::validateIpAddress(const std::string &ipAddress){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

