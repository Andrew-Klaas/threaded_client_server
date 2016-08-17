#Requires OpenSSL and MSGPACKC

cmake_minimum_required(VERSION 3.4.1)
#set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_CXX_STANDARD 14)

project(NPC)
 
include_directories(include)

#OPENSSL
find_package(OpenSSL REQUIRED)
message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
include_directories(${OPENSSL_INCLUDE_DIR})

#MSGPACK, NOTE: may need to change based on your OS
include_directories(/usr/local/lib)
 
file(GLOB SOURCES "src/*.cc")
 
add_executable(testNPC ${SOURCES})

TARGET_LINK_LIBRARIES( testNPC
    pthread
    ssl
    crypto
)
