cmake_minimum_required(VERSION 2.6)

project (ZZG_SERVER)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-9/root/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-9/root/usr/bin/g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -O0 -Wno-unused-variable -pthread")

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
	/usr/lib64/mysql/
    /usr/local/lib/
)

set(base_src
../base/Logger.h
)

set(src
../main.cpp
)


add_executable(MyServer ${src})


