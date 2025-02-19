
# CMake 项目构建指南

## 项目结构
```
cmakeTutorial/
├── CMakeLists.txt
├── main.cpp
└── build/        # 构建目录
```

## 构建步骤

1. 创建并进入构建目录：
```bash
mkdir build
cd build
```

2. 生成构建系统：
```bash
cmake ..
```

3. 编译项目：
```bash
make
```

## 运行程序

构建完成后，在build目录下运行可执行文件：
```bash
./hello
```

## 常见问题

- 如果需要重新构建，可以删除build目录下的所有文件：
```bash
rm -rf build/*
```

- 如果要切换编译类型（比如Debug/Release），可以使用：
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## 注意事项
- 确保系统已安装CMake（版本 >= 3.10）
- 确保build目录与源代码目录分开
- 始终在build目录下执行cmake命令
