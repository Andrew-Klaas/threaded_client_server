#include "node.h"


	

void Node::start(std::string ip, std::string port) {
  //unsigned int nthreads = std::thread::hardware_concurrency();
  Running = true;
	
  // start server thread
  server_thread = std::thread([&] { 
      this->n_server.serve(port, this->pending_ops_q); 
  });

  sleep(2);

	// start client thread
  client_thread = std::thread([&] { 
      this->n_client.serve(ip, port, this->pending_send_q); 
  });
	
	// worker_thread std::thread([&] { this->n_server.serve(port, this->pending_send_q); });

  while(Running){
  }
  server_thread.join();
  client_thread.join();

}

// worker 
//worker threads watching queues
/*
void node::run(){
	  bool running = true;
    while (running) {
      std::lock_guard<std::mutex> lck(storage_mtx);
      auto ops = std::move(rcv_q);
      if (!ops.empty()) {
        printf("runner executing op\n");
        ops.front()();
      }
    } 
}
*/
