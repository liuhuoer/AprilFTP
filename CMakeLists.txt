CMAKE_MINIMUM_REQUIRED(VERSION 2.8) #cmake最低版本要求

PROJECT(AprilFTP)                #工程名CppTemplate

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")    #添加c++11标准支持

SET(CMAKE_C_COMPILER "/usr/bin/gcc")#默认c编译器
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")  #默认C++编译器

SET(CMAKE_BUILD_TYPE "Debug")       #Debug模式 选项：Release Debug

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") #debug模式下gdb相关选项
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


add_definitions(-Ddebug)            #相当于代码中的#define DEBUG

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin") #可执行文件输出目录

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")

# 这段代码可以区分操作系统
MESSAGE("Identifying the OS...")
if(WIN32)
    MESSAGE("This is Windows.")
elseif(APPLE)
    MESSAGE("This is MacOS.")
elseif(UNIX)
    MESSAGE("This is Linux.")
endif()
# 这段代码可以区分操作系统

AUX_SOURCE_DIRECOTORY(../AprilFTP DIR_SRCS) #添加源代码文件夹，自动扫描所有文件

add_executable(chess                #输出名为chess的可执行文件
    ${DIR_SRCS}
)
