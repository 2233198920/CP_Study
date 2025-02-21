
# 定义编译器和编译选项
CC = gcc
CFLAGS = -Wall -O2

# 定义源文件和目标文件
SRCS = main.c utils.c
OBJS = $(SRCS:.c=.o)         # 在引用变量时，变量名前面加$()符号
TARGET = myapp

# 默认目标
all: $(TARGET)

# 链接目标文件
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

# 模式规则：编译.c文件生成.o文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 声明伪目标
.PHONY: all clean
