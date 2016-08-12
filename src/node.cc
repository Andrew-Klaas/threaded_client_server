#include "node.h"

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

      /*
      decltype(recv_q) recv {};
      {
          std::lock_guard<std::mutex> lck(recv_mtx);
          recv = std::move(recv_q);
      }
      */
    }
  });

}

void Node::join() {
  run_thread.join();
  server_thread.join();
  client_thread.join();
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
  packRpcSendReq(rpc, "1", ip, port);
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

/*
void Node::prepareSockRpcSend(rpc_msg& rpc, std::string ip, std::string port);
  rpc->ip = ip;
  rpc->port = port;
  memset(&(rpc->hints), 0, sizeof rpc->hints);
  rpc->hints.ai_family = AF_UNSPEC;
  rpc->hints.ai_socktype = SOCK_STREAM;
}
*/

void Node::packRpcSendReq(rpc_msg& rpc, std::string fn, std::string ip, std::string port){ 
  rpc.ip = ip;
  rpc.port = port;
  rpc.vec.push_back(rpc.ip);    
  rpc.vec.push_back(rpc.port);    
  rpc.vec.push_back(fn); //ping    
  rpc.vec.push_back("");  // No data
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
