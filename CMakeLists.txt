cmake_minimum_required(VERSION 3.20)
project(program)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES}
        Peer.h
        Connection.cpp
        Connection.h)
