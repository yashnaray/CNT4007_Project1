cmake_minimum_required(VERSION 3.20)
project(program)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
