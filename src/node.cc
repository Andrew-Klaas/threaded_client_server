#include "node.h"

Node::Node(int NodeID) : nodeID(NodeID), n_server(NodeID), n_client(NodeID) {};

void Node::start(std::string ip, std::string port) {
  this->ip = ip;
  this->port = port;
  //unsigned int nthreads = std::thread::hardware_concurrency();
  Running = true;
	
  // start server thread
  server_thread = std::thread([&] { 
      this->n_server.serve(port, this->recv_q, recv_mtx); 
  });

  // start client thread
  client_thread = std::thread([&] { 
      this->n_client.serve(ip, port, this->pending_send_q, send_mtx); 
  });
	
  run_thread  = std::thread([&] { 
    while(Running){
      decltype(pending_ops_q) pending_ops {};
      { 
        std::lock_guard<std::mutex> lck(pending_mtx);
        pending_ops = std::move(pending_ops_q);
      }
      while (!pending_ops.empty()) {
        pending_ops.front()(); //whats going on here
        pending_ops.pop();
      }

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
  int result = stoi(args[2]);
   switch(result) {
     case 0:
       break;
     case 1:
       pending_ops_q.emplace([=](){
         sendReplyPeerID(args[0],args[1],stoi(args[3]));
       });
       //cv.notify_all();
       break;
     case 2:
       pending_ops_q.emplace([=](){
        printPeerID(args[5]);
       });
       //cv.notify_all();
       break;
     case 3:
       printf("Reciving hash request\n");
       pending_ops_q.emplace([=](){
         hash_handler(args);
       });
       break;
     default: break;
   }

}
void Node::hash_handler(std::vector<std::string> args){
  if (args[3] == "SHA1") {
    calcSHA1(args);

  } 
  else if (args[3] == "SHA256") {


  }
  else {
    printf("undefined hash function\n");
  }
}

void Node::calcSHA1(std::vector<std::string>& args) {
  std::cout << args[5] << std::endl; 
  unsigned char hash_ptr[sizeof(args[5])]; // == 20
  printf("size of data: %lu \n", sizeof(args[5]));
  
  auto data_ptr = args[5].c_str();
  
  //auto hash_ptr = std::make_unique<unsigned char[]>(sizeof(args[5]));

  SHA1((unsigned char*)data_ptr, sizeof(args[5]), hash_ptr);

  for(auto i = 0 ; i  < sizeof(hash_ptr); i++) {
    printf("%x",hash_ptr[i]);
  }
  printf("\n");

}




void Node::printPeerID(std::string result) {
  std::cout<< "Node: " << nodeID << " PEERID RESULT: " << result << "\n";  
}

std::string Node::ReqPeerID(std::string ip, std::string port, std::size_t length){
  std::lock_guard<std::mutex> lck(pending_mtx); 
  pending_ops_q.emplace([=](){
      sendReqPeerID(ip, port, length);
  });
  //cv.notify_all();
  return "TODO";
}

void Node::sendReqPeerID(std::string ip, std::string port, std::size_t length){

  // fix this
  rpc_msg rpc;
  auto length_s = std::to_string(length);
  std::vector<std::string> args = { length_s, "0", "" };
  packRpcSendReq(rpc, "1", ip, port, args);
  pending_send_q.emplace(std::move(rpc));

}

void Node::sendReplyPeerID(std::string ip, std::string port, std::size_t length){
  printf("Node %d, sending reply\n",nodeID);
  // fix this
  rpc_msg rpc;
  //printf("length is %lu\n", length);
  auto result = random_string(length);
  std::vector<std::string> args = { "", std::to_string(result.length()), result };
  //std::cout << "result is: " << result << std::endl;
  packRpcSendReq(rpc, "2", ip, port, args);
  //pending_send_q.emplace(std::move(rpc));
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
  rpc_msg rpc;
  std::string data_s(data);
  std::vector<std::string> args = { hash_fn, "0", data_s };
  packRpcSendReq(rpc, "3", ip, port, args);
  pending_send_q.emplace(std::move(rpc));

}


void Node::join() {
  run_thread.join();
  server_thread.join();
  client_thread.join();
}


