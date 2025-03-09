#include <sys/socket.h>     // socket相关API
#include <errno.h>          // 错误码定义
#include <netinet/in.h>     // 网络地址结构体

#include <stdio.h>          // 标准输入输出
#include <string.h>         // 字符串处理函数
#include <unistd.h>         // UNIX标准函数

#include <pthread.h>        // 线程相关函数
#include <sys/poll.h>       // poll多路复用
#include <sys/epoll.h>      // epoll多路复用
#include <sys/time.h>       // 时间相关函数


#define BUFFER_LENGTH		512    // 缓冲区大小定义

// 回调函数类型定义：返回值为int，参数为文件描述符
typedef int (*RCALLBACK)(int fd);

// 回调函数前置声明
// 处理新的客户端连接请求
int accept_cb(int fd);
// 处理客户端数据接收
int recv_cb(int fd);
// 处理数据发送
int send_cb(int fd);

// 连接项结构体：保存每个连接的状态和数据
struct conn_item {
	int fd;                             // 文件描述符
	
	char rbuffer[BUFFER_LENGTH];        // 接收缓冲区
	int rlen;                           // 接收数据长度
	char wbuffer[BUFFER_LENGTH];        // 发送缓冲区
	int wlen;                           // 发送数据长度

	// 使用联合体节省内存，因为一个连接不会同时需要accept_callback和recv_callback
	union {
		RCALLBACK accept_callback;      // accept事件回调
		RCALLBACK recv_callback;        // 接收数据回调
	} recv_t;
	RCALLBACK send_callback;            // 发送数据回调
};
// 注：这里的结构类似于libevent库的实现方式

// 全局变量
int epfd = 0;                                      // epoll实例描述符
struct conn_item connlist[1048576] = {0};          // 连接列表，使用文件描述符作为索引
                                                   // 1048576 = 2^20，支持百万级连接
struct timeval zvoice_king;                        // 性能测试用的时间戳

// 计算两个时间差(毫秒)的宏
// 1000000

#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)


int set_event(int fd, int event, int flag) {

	if (flag) { // 1 add, 0 mod
		struct epoll_event ev;
		ev.events = event ;
		ev.data.fd = fd;
		epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
	} else {
	
		struct epoll_event ev;
		ev.events = event;
		ev.data.fd = fd;
		epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
	}

	

}

int accept_cb(int fd) {

	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);

	int clientfd = accept(fd, (struct sockaddr*)&clientaddr, &len);
	
	if (clientfd < 0) {
		return -1;
	}

	printf("accept clientfd: %d\n", clientfd);

	set_event(clientfd, EPOLLIN, 1);

	connlist[clientfd].fd = clientfd;
	memset(connlist[clientfd].rbuffer, 0, BUFFER_LENGTH);
	connlist[clientfd].rlen = 0;
	memset(connlist[clientfd].wbuffer, 0, BUFFER_LENGTH);
	connlist[clientfd].wlen = 0;
	
	connlist[clientfd].recv_t.recv_callback = recv_cb;
	connlist[clientfd].send_callback = send_cb;

	if ((clientfd % 1000) == 999) {
		struct timeval tv_cur;
		gettimeofday(&tv_cur, NULL);
		int time_used = TIME_SUB_MS(tv_cur, zvoice_king);

		memcpy(&zvoice_king, &tv_cur, sizeof(struct timeval));
		
		printf("clientfd : %d, time_used: %d\n", clientfd, time_used);
	}

	return clientfd;
}

int recv_cb(int fd) { // fd --> EPOLLIN

	char *buffer = connlist[fd].rbuffer;
	int idx = connlist[fd].rlen;
	
	// buffer+idx: 讲新数据追加在buffer的尾部
	// BUFFER_LENGTH-idx: 剩余空间大小
	int count = recv(fd, buffer+idx, BUFFER_LENGTH-idx, 0);
	if (count == 0) {
		printf("clientfd: %d close\n", fd);

		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);		
		close(fd);
		
		return -1;
	}
	connlist[fd].rlen += count;

    printf("socketfd: %d recv count: %d --> buffer: %s\n", fd, count, buffer);

	memcpy(connlist[fd].wbuffer, connlist[fd].rbuffer, connlist[fd].rlen);
	connlist[fd].wlen = connlist[fd].rlen;
	connlist[fd].rlen = 0;


	set_event(fd, EPOLLOUT, 0);

	
	return count;
}

int send_cb(int fd) {

	char *buffer = connlist[fd].wbuffer;
	int idx = connlist[fd].wlen;

	int count = send(fd, buffer, idx, 0);

	set_event(fd, EPOLLIN, 0);

	return count;
}


int init_server(unsigned short port) {

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr_in));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	if (-1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr))) {
		perror("bind");
		return -1;
	}

	listen(sockfd, 10);

	return sockfd;
}

// ip:43.133.211.95
int main() {

	int port_count = 20;
	unsigned short port = 2048;
	int i = 0;

	
	epfd = epoll_create(1); // int size

	for (i = 0;i < port_count;i ++) {
		int sockfd = init_server(port + i);  // 2048, 2049, 2050, 2051 ... 2057
		connlist[sockfd].fd = sockfd;
		connlist[sockfd].recv_t.accept_callback = accept_cb;
		set_event(sockfd, EPOLLIN, 1);
	}

	gettimeofday(&zvoice_king, NULL);

	struct epoll_event events[1024] = {0};
	
	while (1) { // mainloop();

		int nready = epoll_wait(epfd, events, 1024, -1); // 

		int i = 0;
		for (i = 0;i < nready;i ++) {

			int connfd = events[i].data.fd;
			if (events[i].events & EPOLLIN) { //

				int count = connlist[connfd].recv_t.recv_callback(connfd);

				if(count == -1) {
					continue;
				}

				//printf("recv count: %d <-- buffer: %s\n", count, connlist[connfd].rbuffer);

			} else if (events[i].events & EPOLLOUT) { 
				printf("send --> buffer: %s\n",  connlist[connfd].wbuffer);
				
				int count = connlist[connfd].send_callback(connfd);
			}

		}

	}


	getchar();
	//close(clientfd);

}




