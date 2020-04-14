cmake_minimum_required(VERSION 3.0)

project(ToyJSON)

include_directories(test)

aux_source_directory(test TEST_LIST)

set(SRC_LIST ./Json.cpp ./test/test.cpp)

# -g3 可以调试宏定义
add_compile_options(-std=c++11 -Wall -O0 -g)

add_executable(testjson.out ${SRC_LIST})