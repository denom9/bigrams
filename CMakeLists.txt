cmake_minimum_required(VERSION 3.8)
project(bigrammi)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fopenmp")

set(SOURCE_FILES main.cpp SeqQueue.h threads.h parameters.h ParQueue.h utilities.h NodeTemplate.h)
add_executable(bigrammi ${SOURCE_FILES})