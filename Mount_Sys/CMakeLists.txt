cmake_minimum_required(VERSION 3.16)
#add_definitions(-std=c++11)

project (sysEthcProject)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-statement-expression")


#守护进程
add_subdirectory(supr)
#添加模块
add_subdirectory(leader) 