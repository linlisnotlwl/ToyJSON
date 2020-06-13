cmake_minimum_required(VERSION 3.0)

project(ToyJSON)

include_directories(test)

aux_source_directory(test TEST_LIST)

set(SRC_LIST_1 ./Json.cpp ./test/test.cpp)

set(SRC_LIST_2 ./test/purestack_test.cpp)

set(SRC_LIB ./Json.cpp)

# -g3 可以调试宏定义
#add_compile_options(-std=c++11 -Wall -O0 -g3)

# release version
add_compile_options(-std=c++11 -Wall -O2)

add_executable(testjson.out ${SRC_LIST_1})
#add_executable(purestacktest.out ${SRC_LIST_2})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_library(toyjson ${SRC_LIB})