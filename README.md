# Client/Server Peer
Development environment:
- Ubuntu 16.04
- g++ (Ubuntu 5.4.0-6ubuntu1~16.04.2) 5.4.0 
- cmake 3.5.1

# Dependencies:
### OpenSSL
```
sudo apt-get install libssl-dev
```

# Compiling
```
git clone git@github.com:Andrew-Klaas/threaded_client_server.git
cd build/
cmake ..
make
```
### MsgpackC
- https://github.com/msgpack/msgpack-c
``` 
$ git clone https://github.com/msgpack/msgpack-c.git
$ cd msgpack-c
$ cmake -DMSGPACK_CXX11=ON .
$ sudo make install
```
## Example:
see src/main.cc
 ```
  Node node1(1);
  Node node2(2);;
  node1.start("127.0.0.1", "3490");
  node2.start("127.0.0.1", "3491");
  ...
  ...
  //Request Random ID 512 bytes long from node2
  node1.ReqPeerID("127.0.0.1","3491", 512 ); 
  
  // Request SHA1 hash of the "blob" data from node1
  node2.ReqHash("127.0.0.1","3490", "SHA1", blob); 
  ```
Example output with Hash of 1 MiB:
```
Node 1, Server Starting on 3490  
Node 2, Server Starting on 3491  
Node 1, RequestID from  127.0.0.1:3491
Node 1, connected to 127.0.0.1:3491 ...sending ID Request 
Node 2, received request for ID
Node 2, connected to 127.0.0.1:3490 ...sending ID Response 
Node 1, ID received: sSsXx5G6XuznAtZd1v0XzmuzyOPAf8SXYK4VPIbuAZhKSEvRAvw7hq6dDUlsaDN8VPeuhDmrmUCCi7dq1axgO2IZW3R4HmCmBofs2RimtszcxcSyAOdZQv8uyYyFK80WxdMx24jwugYqHymRMP0kK9eJhaWzhUVc7sZAuG6nxcdEaPfvped9nGQSqvRVPv7VngdfvjSqJ32rThkGJON4eoWSjwy6r5bckEHdviTClV4CAoSUCqYoc3HJzFPqIzQ2Bhf5N6H7bLJm8kEIam7CpOVmbsctr0u1gX63eNADhSxnCB5kyAwlYPY7F8z59r6pOCq0XzCCR9zbL5LHFF0mcWrrfqwmh0Z4CP2iOEunMuOfxjwAwuwYRmO4cKqIJPMVmOD9b8wx0Jcx0W7xQ2TportOAhgT42wrP8y0GuvEDXBC1G9PIcF4S6Tco75q81fX9dVNWObhtktu12HHdWM5bnfNukD0jtVrUyC0MmiFUb7VcPnDt7ISsxpmf3nMuGBO
Node 2, Requesting SHA1 Hash of 1048576 bytes 
Node 2, connected to 127.0.0.1:3490 ...sending SHA1 Request 
Node 1, received SHA1 request of 1048576 bytes of data from peer ... calculating hash
Node 1, connected to 127.0.0.1:3491 ...sending SHA1 Response 
Node 2, received SHA1 hash: 2d693d25934f7ef6192dca21e7d69c2d6d8f72c
```

## Implementation Notes
see include/node.h and src/node.cc for API and some function explanations

There are three main components to each peer in the design:
a Node, server, and a client. Each node (the peer) has a nested server and client class which watch their respective work queues. Take a look at "Node::start()" in src/node.cc for details. Basically, the start function spawns all threads that watch for events/work to be placed on the queues. The client sends outbound requests, and the server handles inbound requests.


## RPC protocol layout
The RPC messages are implemented using msgpackC and std::vector<std::string>
```
/* example -> std::vector<std::string> msg
 *  msg[0] = Source IP
 *  msg[1] = Source Port
 *  msg[2] = Function to execute-> 
 *     0: Undefined, 1: Request PeerID, 2: PeerID response, 3: Hash request, 4: Hash response
 *  msg[3] = Length of requested PeerID, or the type of hash requested
 *  msg[4] = Size of requested PeerID in bytes, or blob to hash in bytes
 *  msg[5] = The data, example: the random PeerID or the hash value of the blob
 */
 ```

 
 
 
 
 ### MISC:
 Note. a big chunk of the "server" and "client" code were pulled from "Beej's guide to network programming" 
url = http://beej.us/guide/bgnet/

### Another example:
3 nodes:
```
Node 1, Server Starting on 3490  
Node 2, Server Starting on 3491  
Node 3, Server Starting on 3492  
Node 1, RequestID from  127.0.0.1:3491
Node 1, connected to 127.0.0.1:3491 ...sending ID Request 
Node 2, received request for ID
Node 2, connected to 127.0.0.1:3490 ...sending ID Response 
Node 1, ID received: zrO4CpUheZJp5VN0ePiV1W2fxQ9rmgPjXlnjbFPFmg4sCQsopYJo4JSzkbrUFEBn0wUZBtoyYrokHeW4CpsF7KEru3J9ISuGOPpZGbVoSJWhx2jAqcNxubmof3wvVqBtD0SUbwG2FnhBnRLd1gYtFKiuNcnsQyjewC6W6KWL5DWtcpUbT3VjLBbjnPZBLJpITtmXEIsJTNA4Adfcg8LzJwg4JGGfX3vyxfV9vLQMgbQrE4RsCmrUgYY0modJqWFnBku65KSmtrb5t2x3mnXSL4Q5s1MgXbRgLLkOeB8X2jdtjYwTJSucWIhOK33peUWxqGMURUzRCaKt9FLQhF3DXkZrmchQ7BMxRgPt8OIKzdC6sXWZkZkHIJ74wmS3xmyMSNDbjWti73mzaJWIqGa6XfATRbUMNQiqlwPVQIBXMwUuD0D3El8lOGDnrf9D6p1pjQI9gUg0OAtb84cKnk4B1Fyqu73yu2meQ4l5YQ5wYwVh08zns3wrIuiB1j7umtYA
Node 3, Requesting SHA256 Hash of 1048576 bytes 
Node 3, connected to 127.0.0.1:3490 ...sending SHA256 Request 
Node 2, Requesting MD5 Hash of 1048576 bytes 
Node 2, connected to 127.0.0.1:3490 ...sending MD5 Request 
Node 1, received SHA256 request of 1048576 bytes of data from peer ... calculating hash
Node 1, connected to 127.0.0.1:3492 ...sending SHA256 Response 
Node 3, received SHA256 hash: 11c056eaa924e280df51c405c436c12ab313422fa8b65f2fbb197faf6ac154
Node 1, received MD5 request of 1048576 bytes of data from peer ... calculating hash
Node 1, connected to 127.0.0.1:3491 ...sending MD5 Response 
Node 2, received MD5 hash: 9e7e82781a4e653feb98811f9a745ea
Node 1, Shutting down
Node 2, Shutting down
Node 3, Shutting down
```



