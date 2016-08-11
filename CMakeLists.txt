
cmake_minimum_required(VERSION 3.4.1)
#set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_CXX_STANDARD 14)

project(NPC)
 
#Bring the headers, such as Student.h into the project
include_directories(include)
include_directories(/usr/include)
include_directories(/usr/local/include)
include_directories(/usr/local/lib)
#include_directories(/usr/local/Cellar/msgpack/1.4.2/include)
 
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")
 
add_executable(testNPC ${SOURCES})

TARGET_LINK_LIBRARIES( testNPC
    pthread
)
