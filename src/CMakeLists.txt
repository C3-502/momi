cmake_minimum_required(VERSION 2.8)

project(momi)

add_executable(${PROJECT_NAME} main.cpp momi.cpp util.cpp connection.cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2 -L/usr/local/lib -lcurl -pthread")
