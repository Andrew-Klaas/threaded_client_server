#include "node.h"
#include "unistd.h"


int main(int argc, char* argv[]){
  
  Node node1(1);
  Node node2(2);;
 
  node1.start("127.0.0.1", "3490");
  node2.start("127.0.0.1", "3491");

  sleep(1);

  auto test = "0123456789k;laskefj;lwkejfwl;ekf";
  node2.ReqHash("127.0.0.1","3490", "SHA1", test);

  /*
  node2.ReqPeerID("127.0.0.1","3490", 1024 );
  sleep(1);
  node2.ReqPeerID("127.0.0.1","3490", 25 );
  sleep(1);
  node1.ReqPeerID("127.0.0.1","3491", 5 );
  */

  //node2.ReqPeerID("127.0.0.1","3490", length );
  //node2.ReqPeerID("127.0.0.1","3490", length );


  while(1){}

  node1.join();
  node2.join();
  
  return 0;
};

//TODO
/*
   the pending ops queue won't work for the return value in the outgoing part teh
     way i'm thinking it will

   add NodeID 
   
   set up worker node
   handle msg packing
   handle msg unpacking
   lookup notify_all
   add locks for queue accesses
   implement RPC mechanism 
    case statments
    lambda functions

   Request Peer ID
    In this request, a peer asks its remote to respond with a unique ID in the
    form of a variable length string of characters

   Request Hash
    in this request, a peer sends a block of data and one of a set of hash functions
    The peer must calculate teh hash of the data, and then respond with the 
    resultant hash.

   leftover data sending, Beej's guide

//rcv should keep the connection information
// So i need to have a function that sets up network structs
// then packs those and information into a request queue.
// then server pulls off queue and does a send
  
  Send out the RPC, then wait for reply in receive queue

  valgrind
    
*/
