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


// 线程处理函数：处理单个客户端连接的数据收发
void *client_thread(void *arg) {
    // 从参数中获取客户端socket描述符
    int clientfd = *(int*)arg;

    while(1) {
        char buf[128] = {0};  // 初始化接收缓冲区
        // 接收客户端数据
        int count = recv(clientfd, buf, sizeof(buf), 0);
        if(count == 0) {  // 客户端断开连接
            break;
        }
        // 将接收到的数据发送回客户端（回显服务）
        send(clientfd, buf, count, 0);
        printf("收到数据 - clientfd: %d, count: %d, buf: %s\n", clientfd, count, buf);
    }
    close(clientfd);  // 关闭客户端连接
    return NULL;
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
// 方案1：基础阻塞式单线程处理
// 特点：一次只能处理一个客户端连接，其他客户端需要等待

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


	
#elif 0  
// 方案2：多线程并发处理
// 特点：每个客户端连接都由独立线程处理，支持并发连接
// 缺点：线程资源开销大，并发量受限于系统线程数

while(1){

		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);

		int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

		pthread_t thid;
		pthread_create(&thid, NULL, client_thread, &clientfd);
		/*
		&thid：指向线程标识符的指针，用于存储新创建线程的ID
		NULL：线程属性，NULL表示使用默认属性
		client_thread：线程将要执行的函数
		&clientfd：传递给线程函数的参数
		*/
	}




#elif 0 
// 方案3：select多路复用
// 特点：单线程同时处理多个连接，最大支持1024个文件描述符
// 原理：通过fd_set数据结构来跟踪多个文件描述符的状态
// int nready = select(maxfd, rset, wset, eset, timeout);
/* 		正值 - 代表已准备好的文件描述符数量（即：返回发生了状态变化的文件描述符个数）
		0 - 超时时间到达，但没有任何文件描述符准备就绪
		-1 - 发生错误，此时应该检查 errno 来确定具体的错误原因	
*/

	fd_set rfds,rset;   //定义读文件描述符集合
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);  //将sockfd加入到rfds中
	int maxfd = sockfd;   //最大文件描述符3
	
	while(1){

		rset = rfds;
		int nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(sockfd, &rset)){

			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			
			FD_SET(clientfd, &rfds);
			maxfd = clientfd;
		

		}

		int i = 0;
		for(i = sockfd+1; i <= maxfd; i++){
			if(FD_ISSET(i, &rset)){
				char buf[128] = {0};  
				int count = recv(i, buf, sizeof(buf), 0);
				if(count == 0 ){
					printf("disconnect - clientfd: %d\n", i);
					FD_CLR(i, &rfds);
					close(i);
				}
				else{
					send(i, buf, count, 0);
					printf("收到数据 - clientfd: %d, count: %d, buf: %s\n", i, count, buf);
				}
			}
		}
			

	}



#elif 0	
// 方案4：poll多路复用
// 特点：功能与select类似，但没有文件描述符数量限制
// 优势：性能比select略好，不需要重新设置监听集合
// int poll(struct pollfd *fds, nfds_t nfds, int timeout);
/*
	fds：pollfd结构体数组
	nfds：数组中的元素个数
	timeout：超时时间（毫秒），-1表示永久阻塞
*/

	struct pollfd fds[1024];

	fds[sockfd].fd = sockfd;
	fds[sockfd].events = POLLIN;   //POLLIN: 监听文件描述符的读事件。

	int maxfd = sockfd;
	
	while(1){

		int nready = poll(fds, maxfd+1, -1);

		//使用位与运算(&)检查 revents 中是否包含 POLLIN 事件
		if(fds[sockfd].revents & POLLIN){    
			
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

			fds[clientfd].fd = clientfd;
			fds[clientfd].events = POLLIN;
			maxfd = clientfd;
		}
		
		int i = 0;
		for(i = sockfd+1; i <= maxfd; i++){

			if(fds[i].revents & POLLIN){
				char buf[128] = {0};  
				int count = recv(i, buf, sizeof(buf), 0);
				if(count == 0 ){
					printf("disconnect - clientfd: %d\n", i);
					fds[i].fd = -1;
					close(i);
					continue;
				}
				else {
					send(i, buf, count, 0);
					printf("收到数据 - clientfd: %d, count: %d, buf: %s\n", i, count, buf);
				}
			}
				

		}



	}


#elif 1
// 方案5：epoll多路复用
// 特点：Linux特有的高性能I/O多路复用机制
// 优势：支持高并发、不受文件描述符限制、性能最优

    // 创建epoll实例，参数值只需>0即可
    int epfd = epoll_create(1); 

    // 配置事件结构体
    struct epoll_event ev;
    ev.events = EPOLLIN;  // 监听读事件
    ev.data.fd = sockfd;  // 设置监听的文件描述符

    // 将监听socket添加到epoll实例
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    // 事件数组，用于接收发生的事件
    struct epoll_event events[1024] = {0};
    
    while (1) {
        // 等待事件发生，-1表示永久阻塞
        int nready = epoll_wait(epfd, events, 1024, -1);

        // 处理所有就绪的事件
        for (int i = 0; i < nready; i++) {
            int connfd = events[i].data.fd;
            
            if (sockfd == connfd) { 
				//基于IO事件驱动，也可设计基于不同的IO类型


                // 接受新连接
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

                // 设置新连接为边缘触发模式
                ev.events = EPOLLIN | EPOLLET;  
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

                printf("新客户端连接 - clientfd: %d\n", clientfd);

            } else if (events[i].events & EPOLLIN) {  // 已连接客户端有数据可读
                // 读取客户端数据（这里故意使用小缓冲区来演示触发机制的区别）
                char buffer[5] = {0};
                int count = recv(connfd, buffer, 5, 0);

                if (count == 0) {  // 客户端断开连接
                    printf("客户端断开连接\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);		
                    close(i);
                    continue;
                }
                
                // 回显数据给客户端
                send(connfd, buffer, count, 0);
                printf("收到数据 - clientfd: %d, count: %d, buffer: %s\n", 
                       connfd, count, buffer);
            }
        }
    }
#endif
    return 0;
}

