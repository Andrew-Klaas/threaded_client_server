#include "node.h"
#include "unistd.h"

std::string test_random_string( std::size_t length );

int main(int argc, char* argv[]){
  
  Node node1(1);
  Node node2(2);;
 
  node1.start("127.0.0.1", "3490");
  node2.start("127.0.0.1", "3491");

  sleep(1);

  auto test = "0123456789";

  


  auto test_large = test_random_string(50000); 
  //std::cout << test_large << std::endl;
  node2.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
  //node1.ReqHash("127.0.0.1","3490", "SHA256", test);
  //node2.ReqHash("127.0.0.1","3490", "SHA1", test);
  //node1.ReqHash("127.0.0.1","3490", "SHA1", test);
  //node2.ReqHash("127.0.0.1","3490", "SHA1", test);

  /*
  node2.ReqPeerID("127.0.0.1","3490", 1024 );
  sleep(1);
  node2.ReqPeerID("127.0.0.1","3490", 25 );
  sleep(1);
  node1.ReqPeerID("127.0.0.1","3491", 5 );
  */

  //node2.ReqPeerID("127.0.0.1","3490", length );
  //node2.ReqPeerID("127.0.0.1","3490", length );

  sleep(1);
  //printf("\n%lu\n", sizeof(unsigned long));

  node1.join();
  node2.join();
  
  return 0;
};


std::string test_random_string( std::size_t length ) {
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


//TODO
/*
   lookup notify_all
   add locks for queue accesses
   implement RPC mechanism 
   - use struct for RPC? Golang style  
   lambda functions
   rcv should keep the connection information
   So i need to have a function that sets up network structs
   then packs those and information into a request queue.
   then server pulls off queue and does a send
   Send out the RPC, then wait for reply in receive queue
   valgrind
   memory copying, pass by more refs instead?
   general refactor
    
*/
