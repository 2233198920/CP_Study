# CMake 项目构建指南

## 1. 项目结构
```
cmakeTutorial/
├── CMakeLists.txt   # 主构建配置文件
├── main.cpp         # 主程序源文件
└── build/           # 构建输出目录
```

## 2. 构建指南
### 基本构建步骤
```bash
mkdir build && cd build  # 创建并进入构建目录
cmake ..                 # 生成构建系统
make                     # 编译项目
./hello                  # 运行程序
```

### 常用构建选项
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..  # 切换为Debug构建
rm -rf build/*                     # 清理构建目录
```

## 3. CMake 基础知识
### 重要路径变量
- `PROJECT_BINARY_DIR`: 构建目录路径
- `PROJECT_SOURCE_DIR`: 源代码根目录路径
- `CMAKE_CURRENT_SOURCE_DIR`: 当前CMakeLists.txt所在路径

### 简单多目录项目示例
```cmake
cmake_minimum_required(VERSION 3.10)
project(HelloCMake LANGUAGES CXX)

# 配置安装路径
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# 子目录配置
set(SUB_DIR_LIST dir1 dir2)
set(SRC_LIST main.cpp)

# 添加子目录源文件
foreach(SUB_DIR ${SUB_DIR_LIST})
    include_directories(${SUB_DIR})
    aux_source_directory(${SUB_DIR} SRC_LIST)
endforeach()

# 构建和安装配置
add_executable(hello ${SRC_LIST})
install(TARGETS hello RUNTIME DESTINATION bin)

# 头文件安装配置
set(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/dir1/calc_add.h
    ${CMAKE_SOURCE_DIR}/dir2/calc_multiply.h
)
install(FILES ${HEADER_FILES} DESTINATION include)
```

## 注意事项
- CMake版本要求: >= 3.10
- CMakeLists.txt 文件名区分大小写
- 建议使用外部构建（在build目录中构建）
- 始终在build目录下执行cmake命令
