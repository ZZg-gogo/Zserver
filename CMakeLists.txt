cmake_minimum_required(VERSION 2.6)

project (ZZG_SERVER)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-9/root/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-9/root/usr/bin/g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -ggdb -Wall -O0 -Wno-unused-variable -pthread -lyaml-cpp")

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
	/usr/lib64/mysql/
    /usr/local/lib/
)

 
find_package(Boost 1.53.0 COMPONENTS system filesystem REQUIRED)
 
include_directories(${Boost_INCLUDE_DIRS})

set(base_src
../base/Logger.cpp
../base/util.cpp
../base/Config.cpp
)

set(src
../main.cpp
)


add_executable(MyServer ${src} ${base_src})
target_link_libraries(MyServer ${Boost_LIBRARIES})
target_link_libraries(MyServer PRIVATE yaml-cpp)


