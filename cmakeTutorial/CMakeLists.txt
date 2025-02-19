cmake_minimum_required(VERSION 3.10)

# 工程名
project(HelloCMake LANGUAGES CXX)



# 设置安装路径为项目目录下的install文件夹
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# 相当于Makefile中的SRCS = main.c，就是把main.cpp放进srcs中
set(srcs main.cpp)

# 添加子项目头文件
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/dir1")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR ->" ${CMAKE_CURRENT_SOURCE_DIR})
# link_directories()

# 添加子目录
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dir1")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/dir2")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dir2")

# link_directories("${CMAKE_CURRENT_SOURCE_DIR}/dir1")
# link_directories("${CMAKE_CURRENT_SOURCE_DIR}/dir2")


# 编译主程序
add_executable(hello ${srcs})


# 动态库.so 静态库.a
# libdir1.a libdir2.a 可以去掉lib写为dir
# 先讲dir中的代码编译为libdir.a库文件，然后连接到主函数
# target_link_libraries 是强制找静态库
# 链接库和主程序
target_link_libraries(hello dir1 dir2)

# 安装可执行文件
install(TARGETS hello RUNTIME DESTINATION bin)

# 安装库文件
install(TARGETS dir1 dir2
        ARCHIVE DESTINATION lib)

# 安装头文件
install(FILES 
        ${CMAKE_SOURCE_DIR}/dir1/calc_add.h
        ${CMAKE_SOURCE_DIR}/dir2/calc_multiply.h
        DESTINATION include)