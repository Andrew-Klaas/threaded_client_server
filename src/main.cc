#include "node.h"
#include "unistd.h"

std::string test_random_string( std::size_t length );

int main(int argc, char* argv[]){
  
	//Basic testing to be a similar output match to the Spec
  Node node1(1);
  Node node2(2);
	//Node node3(3);
  auto ready1 = node1.start("127.0.0.1", "3490");
  auto ready2 = node2.start("127.0.0.1", "3491");
	//auto ready3 = node3.start("127.0.0.1", "3492");

  
  node1.ReqPeerID("127.0.0.1","3491", 512 ); //Request Random ID 512 bytes long from node2
	
	// generate 1MiB random string to hash
	auto test_large = test_random_string(1048576); 
	auto test_large2 =  test_random_string(1048577); 
  
	node2.ReqHash("127.0.0.1","3490", "MD5", test_large.c_str()); //Request SHA1 hash of test_large blob 
	node2.ReqHash("127.0.0.1","3490", "SHA256", test_large2.c_str()); //Request SHA1 hash of test_large blob 
	//node3.ReqHash("127.0.0.1","3490", "SHA1", "123456789); //Request SHA1 hash of test_large blob 

	//Let functions execute
	sleep(1);

  node1.join();
  node2.join();
	//node3.join();
  
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
   lookup notify_all: http://en.cppreference.com/w/cpp/thread/condition_variable/notify_all
*/

