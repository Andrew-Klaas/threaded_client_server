#include "node.h"

Node::Node(int NodeID) : nodeID(NodeID), n_server(NodeID), n_client(NodeID) {};

void Node::start(std::string ip, std::string port) {
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

  /*
  for (auto& i : args) {
    std::cout << i << std::endl;
  }
  */
  std::lock_guard<std::mutex> lck(pending_mtx); 
  int result = stoi(args[2]);
  //printf("result is %d\n", result);

   switch(result) {
     case 0:
       //printf("0\n");
       //printf("handler function case 0\n");
       break;
     case 1:
       //printf("1\n");
       pending_ops_q.emplace([=](){
           sendReplyPeerID(args[0],args[1],stoi(args[3]));
       });
       //cv.notify_all();
       break;
     case 2:
       //printf("2\n");
       pending_ops_q.emplace([=](){
       printPeerID(args[3]);
     });
       //cv.notify_all();
       break;
     default: break;
   }
   //printf("undefined function\n");

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
  std::vector<std::string> args = { length_s };
  packRpcSendReq(rpc, "1", ip, port, args);
  pending_send_q.emplace(std::move(rpc));

}

void Node::sendReplyPeerID(std::string ip, std::string port, std::size_t length){
  // fix this
  rpc_msg rpc;
  //printf("length is %lu\n", length);
  auto result = random_string(length);
  std::vector<std::string> args = { result };
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
  rpc.vec.push_back(rpc.ip);    
  rpc.vec.push_back(rpc.port);    
  rpc.vec.push_back(fn); //ping    
  rpc.vec.push_back(args[0]);  // No data
  msgpack::pack(rpc.buffer, rpc.vec);
}

/*
unsigned char *ReqHash(std::string ip, std::string port, string fn, const unsigned char & data){
  //std::lock_guard<std::mutex> lck(storage_mtx); 
  pending_ops_q.emplace([=](Worker& worker){
      worker.sendReqHash(ip, port, fn,data);
  });
  //cv.notify_all();
}
*/

void Node::join() {
  run_thread.join();
  server_thread.join();
  client_thread.join();
}


