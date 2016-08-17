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

  auto test_large = test_random_string(1024*1024); 

	sleep(1);
  node1.ReqPeerID("127.0.0.1","3491", 25 );
	sleep(1);
  node2.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	/*
	node2.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node2.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node2.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node2.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
  node1.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node1.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node1.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node1.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	node1.ReqHash("127.0.0.1","3490", "SHA1", test_large.c_str());
	*/

  //node1.ReqHash("127.0.0.1","3490", "SHA256", test);
  //node2.ReqHash("127.0.0.1","3490", "SHA1", test);
  //node1.ReqHash("127.0.0.1","3490", "SHA1", test);
  //node2.ReqHash("127.0.0.1","3490", "SHA1", test);

  //node2.ReqPeerID("127.0.0.1","3490", 1024 );
  //node2.ReqPeerID("127.0.0.1","3490", 25 );
  //node1.ReqPeerID("127.0.0.1","3491", 512 );

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
	 Add documentation on encapsulation
	 clean up header files
	 clean up cmakelist
   lookup notify_all: http://en.cppreference.com/w/cpp/thread/condition_variable/notify_all
	 error checking/handling
   valgrind
   memory copying, pass by more refs instead?
*/

