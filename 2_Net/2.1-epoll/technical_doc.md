# TCP服务器多路I/O复用技术实现文档

## 1. 概述

本文档详细说明了TCP服务器的四种不同实现方式：
1. 基础阻塞式单线程处理
2. 多线程并发处理
3. Select/Poll多路复用
4. Epoll高性能事件驱动

## 2. 基础架构

所有实现方案共享的基础设施代码：

```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in serveraddr;
// 配置服务器地址...
bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));
listen(sockfd, 10);
```

## 3. 实现方案详解

### 3.1 基础阻塞式处理
- 实现方式：单线程循环处理一个客户端
- 优点：实现简单，适合学习理解TCP通信流程
- 缺点：同一时间只能服务一个客户端
- 关键代码结构：
```c
while(1) {
    int clientfd = accept(sockfd, ...);
    while(1) {
        recv(clientfd, ...);
        send(clientfd, ...);
    }
}
```

### 3.2 多线程并发处理
- 实现方式：每个客户端分配一个独立线程
- 优点：可以并发处理多个客户端连接
- 缺点：线程资源开销大，连接数受限于系统线程数
- 关键函数：`pthread_create()`

### 3.3 Select模型
- 特点：
  - 使用fd_set数据结构记录待监控的文件描述符
  - 有1024个文件描述符的限制
  - 每次调用都需要重新设置fd_set
- 关键API：
  ```c
  int select(int maxfd, fd_set *readfds, fd_set *writefds, 
             fd_set *exceptfds, struct timeval *timeout);
  ```

### 3.4 Poll模型
- 特点：
  - 使用pollfd结构数组管理文件描述符
  - 没有文件描述符数量限制
  - 效率比select略高
- 关键API：
  ```c
  int poll(struct pollfd *fds, nfds_t nfds, int timeout);
  ```

### 3.5 Epoll模型
- 特点：
  - 使用事件驱动机制
  - 支持边缘触发(ET)和水平触发(LT)
  - 高并发场景下性能最优
- 关键API：
  ```c
  int epoll_create(int size);
  int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
  int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
  ```

## 4. Epoll触发模式详解

### 4.1 水平触发（Level Triggered，LT）

水平触发是epoll的默认工作模式。

#### 特点：
- 只要文件描述符上有数据可读，每次调用epoll_wait都会通知
- 可以不一次性把数据读完
- 不会漏掉任何数据
- CPU使用效率较低

#### 工作流程：
1. 读缓冲区有数据时，触发通知
2. 每次read操作不必读完所有数据
3. 只要还有数据，下次epoll_wait仍会通知
4. 类似于select和poll的工作方式

#### 示例代码：
```c
// 水平触发模式下的读取操作
if (events[i].events & EPOLLIN) {
    char buf[1024] = {0};
    int n = read(fd, buf, sizeof(buf));
    // 即使没读完也没关系，剩余数据下次会被通知
    process_data(buf, n);
}
```

### 4.2 边缘触发（Edge Triggered，ET）

边缘触发是一种高效的工作模式，但使用时需要更谨慎。

#### 特点：
- 只有当状态发生变化时才会通知
- 必须一次性把数据读完
- 可能会漏掉数据，需要小心使用
- CPU使用效率高

#### 工作流程：
1. 当读缓冲区状态从无数据变为有数据时，触发一次通知
2. 必须循环读取，直到返回EAGAIN错误
3. 后续数据到达才会再次触发通知
4. 通常需要配合非阻塞I/O使用

#### 示例代码：
```c
// 边缘触发模式下的读取操作
if (events[i].events & EPOLLIN) {
    while (1) {
        char buf[1024] = {0};
        int n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EAGAIN) // 已读完所有数据
                break;
            // 处理其他错误
            break;
        }
        process_data(buf, n);
    }
}
```

### 4.3 两种模式的对比

| 特性 | 水平触发(LT) | 边缘触发(ET) |
|------|-------------|--------------|
| 通知时机 | 只要有数据就通知 | 状态变化时通知 |
| 数据处理 | 可以分多次处理 | 必须一次性处理完 |
| 编程难度 | 简单 | 较复杂 |
| CPU效率 | 较低 | 较高 |
| 适用场景 | 对实时性要求高 | 高并发、高性能场景 |

### 4.4 使用建议

1. **选择触发模式：**
   - 新手建议使用LT模式
   - 追求性能时使用ET模式
   - 对实时性要求高的场景用LT模式

2. **ET模式注意事项：**
   - 必须使用非阻塞I/O
   - 要一次性读完所有数据
   - 注意处理EAGAIN错误

3. **最佳实践：**
   - 建议在ET模式下使用非阻塞I/O
   - 读写操作要采用循环方式
   - 注意保存和处理剩余数据
   - 合理设置缓冲区大小

## 5. 详细运行流程

### 5.1 基础阻塞式模型
```
初始化流程:
socket() → bind() → listen()

运行流程:
while(1) {
    accept() → 阻塞等待连接
    while(1) {
        recv() → 阻塞等待数据
        处理数据
        send() → 发送响应
    }
    close() → 关闭连接
}
```

核心函数：
- `accept()`：阻塞等待客户端连接
- `recv()`：阻塞式接收数据
- `send()`：发送数据响应

### 5.2 多线程模型
```
主线程流程:
socket() → bind() → listen()
while(1) {
    accept() → 获取新连接
    pthread_create() → 创建新线程
}

子线程流程:
while(1) {
    recv() → 接收数据
    处理数据
    send() → 发送响应
}
close() → 关闭连接
```

核心函数：
- `pthread_create()`：创建新线程
- `pthread_detach()`：分离线程
- `pthread_exit()`：线程退出

### 5.3 Select模型
```
初始化流程:
socket() → bind() → listen()
FD_ZERO() → 初始化文件描述符集合
FD_SET() → 添加监听套接字

运行流程:
while(1) {
    select() → 等待事件
    if(监听套接字就绪) {
        accept() → 接受新连接
        FD_SET() → 添加新描述符
    }
    for(所有连接) {
        if(数据就绪) {
            recv() → 接收数据
            处理数据
            send() → 发送响应
        }
    }
}
```

核心函数：
- `FD_ZERO()`：清空描述符集合
- `FD_SET()`：添加描述符到集合
- `FD_CLR()`：从集合移除描述符
- `FD_ISSET()`：测试描述符是否在集合中
- `select()`：等待事件发生

### 5.4 Poll模型
```
初始化流程:
socket() → bind() → listen()
初始化pollfd结构体数组

运行流程:
while(1) {
    poll() → 等待事件
    if(监听套接字就绪) {
        accept() → 接受新连接
        添加新描述符到数组
    }
    for(所有连接) {
        if(数据就绪) {
            recv() → 接收数据
            处理数据
            send() → 发送响应
        }
    }
}
```

核心函数：
- `poll()`：等待事件
- `pollfd结构体`：
  ```c
  struct pollfd {
      int fd;         // 文件描述符
      short events;   // 监控的事件
      short revents;  // 返回的事件
  };
  ```

### 5.5 Epoll模型
```
初始化流程:
socket() → bind() → listen()
epoll_create() → 创建epoll实例
epoll_ctl() → 添加监听套接字

运行流程:
while(1) {
    epoll_wait() → 等待事件
    for(就绪的事件) {
        if(是监听套接字) {
            accept() → 接受新连接
            epoll_ctl() → 添加新描述符
        } else {
            recv() → 接收数据
            处理数据
            send() → 发送响应
        }
    }
}
```

核心函数：
- `epoll_create()`：创建epoll实例
- `epoll_ctl()`：控制epoll实例
  - EPOLL_CTL_ADD：添加监控
  - EPOLL_CTL_MOD：修改监控
  - EPOLL_CTL_DEL：删除监控
- `epoll_wait()`：等待事件
- `epoll_event结构体`：
  ```c
  struct epoll_event {
      uint32_t events;    // 事件类型
      epoll_data_t data;  // 用户数据
  };
  ```



# Epoll百万并发

## 文件描述符构成

- (源ip，目标ip，源端口，目标端口，协议)

## 注意事项
- Cannot assign requested address: 文件描述符fd分配数量不足，启用多个ip
- 需要更改进程控制的最大文件连接符（文件描述符）数量：ulimit -n 用于查看当前数量
- 更改net.ipv4.ip_local_port_range = 1024 65536
- 设置最大文件描述符的值 fs.file-max = 1048576
- 内核读/写协议栈net.ipv4.tcp_rmem/wmem = 1024 1024 2048   （中间为缺省值）
- 内核协议栈总大小（页：4k = 1页）net.ipv4.tcp_mem = 262144 524288 786432 （1g 2g 3g）