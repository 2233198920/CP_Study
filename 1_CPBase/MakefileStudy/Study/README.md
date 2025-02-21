# Makefile 学习指南

## 1. Makefile 基本概念

Makefile是一个用于自动化编译和构建项目的工具。它主要用于：
- 自动化编译过程
- 管理项目依赖关系
- 只重新编译修改过的文件

## 2. Makefile 基本语法

### 2.1 基本规则
```makefile
目标: 依赖
    命令
```

### 2.2 变量定义
```makefile
CC = gcc                # 编译器
CFLAGS = -Wall -g       # 编译选项
```

### 2.3 常用符号
- `$@`: 表示目标
- `$^`: 表示所有依赖
- `$<`: 表示第一个依赖
- `%`: 通配符
- `wildcard`: 通配符函数    SRCS = $(wildcard *.c)   //将所有.c文件加入到SRCS 
- `patsubst`：字符替换      OBJS = $(patsubst %.c,%.o,$(SRCS))  //讲SRCS中的.c替换为.o
                            $(SRCS:.c=.o)   //更简洁
## 3. 常用命令

- `make`: 执行默认目标
- `make clean`: 清理编译产物
- `make -n`: 显示要执行的命令但不实际执行
- `make -f filename`: 指定makefile文件名

## 4. 实用技巧

### 4.1 伪目标
```makefile
.PHONY: clean
clean:
    rm -f *.o
```

### 4.2 自动变量
```makefile
%.o: %.c
    $(CC) -c $(CFLAGS) $< -o $@
```

### 4.3 条件判断
```makefile
ifeq ($(CC),gcc)
    CFLAGS += -Wall
endif
```
