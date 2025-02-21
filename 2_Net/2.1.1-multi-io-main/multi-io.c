// 引入必要的头文件
#include <sys/socket.h>     // socket相关函数声明
#include <errno.h>          // 错误号定义
#include <netinet/in.h>     // IPv4地址结构体定义
#include <stdio.h>          // 标准输入输出
#include <string.h>         // 内存操作函数
#include <unistd.h>         // UNIX标准函数定义
#include <pthread.h>        // 线程相关定义
#include <sys/poll.h>       // poll函数相关定义  
#include <sys/epoll.h>      // epoll函数相关定义


void *client_thread(void *arg) {

	int clientfd = *(int*)arg;

	while(1){

		char buf[128] = {0};  
		int count = recv(clientfd, buf, sizeof(buf), 0);
		if(count == 0 ){
			break;
		}

		send(clientfd, buf, count, 0);
		printf("收到数据 - clientfd: %d, count: %d, buf: %s\n", clientfd, count, buf);

	}
	close(clientfd);

}

// TCP服务器示例程序
int main() {
    // 创建TCP socket
    // AF_INET: IPv4协议族
    // SOCK_STREAM: 面向连接的TCP协议
    // 0: 使用默认协议
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 定义服务器地址结构体并初始化为0
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));

    // 配置服务器地址信息
    serveraddr.sin_family = AF_INET;                // 使用IPv4地址
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡接口
    serveraddr.sin_port = htons(2048);              // 监听2048端口，需要转换为网络字节序

    // 绑定socket与地址
    // 如果绑定失败，打印错误信息并退出
    if (-1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr))) {
        perror("bind");    // 输出错误原因
        return -1;
    }

    // 开始监听连接请求
    // 10表示待处理连接请求队列的最大长度
    listen(sockfd, 10);
#if 0
	while(1) {
		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);
		
		int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
		printf("新客户端连接成功\n");
	
		// 添加内层循环，持续接收当前客户端的数据
		while(1) {
			char buf[128] = {0};  // 清空缓冲区
			int count = recv(clientfd, buf, sizeof(buf), 0);  
			
			if(count > 0) {
				// 正常接收到数据
				printf("收到数据 - sockfd:%d, clientfd: %d, count: %d, buf: %s\n", 
					   sockfd, clientfd, count, buf);
				send(clientfd, buf, count, 0);  // 原样返回数据
			}
			else if(count == 0) {
				// 客户端主动断开连接
				printf("客户端断开连接\n");
				break;
			}
			else {
				// 接收错误
				perror("recv");
				break;
			}
		}
	
		close(clientfd);  // 内层循环结束后关闭客户端连接
	}
#else  //来一个客户端就创建一个线程处理
	while(1){

		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);

		int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

		pthread_t thid;
		pthread_create(&thid, NULL, client_thread, &clientfd);
	}

#endif

    return 0;
}

