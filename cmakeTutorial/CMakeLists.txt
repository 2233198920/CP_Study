cmake_minimum_required(VERSION 3.10)

# 工程名
project(HelloCMake LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 
set(src main.cpp)

# 添加可执行文件
add_executable(hello ${src})
