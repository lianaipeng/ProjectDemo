
## CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8)


## 项目信息
set(NAME projectdemo)
project(${NAME})

message("########## ${CMAKE_CURRENT_SOURCE_DIR}")


## 编译信息
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -std=c++11")


## 头文件信息
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/hiredis-0.13.3  
        ${CMAKE_CURRENT_SOURCE_DIR}/include/curl-7.61.0 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/librdkafka-0.11.5 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/public 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/base 
        )

## 库文件信息
#link_directories(/home/lian.aipeng/work/DispatchService/lib)
link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/hiredis-0.13.3 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/curl-7.61.0 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/librdkafka-0.11.5 
        )
link_libraries(
        hiredis 
        curl 
        rdkafka 
        )


## 源文件信息
## 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src DIR_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/public DIR_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/base DIR_SRCS)
#aux_source_directory(src/ipip DIR_SRCS)
#message("########## ${DIR_SRCS}")

#set(CMAKE_EXE_LINKER_FLAGS "-lpthread -lhiredis")
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")

## 指定生成可执行程序的路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

## 指定生成目标
add_executable(${NAME} ${DIR_SRCS})
#target_link_libraries(${NAME} libhiredis.a)
#target_link_libraries(${NAME} ${EXTRA_LIBS})
