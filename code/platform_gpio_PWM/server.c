
//开发板执行
//arm-hisiv300-linux-gcc server.c -o server
// ./server /dev/car_drver

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
//#include <iostream.h>

#define PORT 7000   //端口
#define QUEUE 20    //监听序号
 
int main(int argc, char *argv[])
{
    fd_set rfds;    //文件描述符集合
    struct timeval tv;  //超时时间结构体
    int retval, maxfd;

    int fd, retvalue;
 	char *filename;
 	unsigned char databuf[1];

	//输入的参数应该要为2个，0、1
	if(argc != 2){
	printf("输入的参数数量不对!\r\n");
	return -1;
	}
	//第二个参数是驱动的节点文件 /dev/cardriver
	filename = argv[1];
	/* 打开 gpio 驱动 */
	fd = open(filename, O_RDWR); //调用open函数
	if(fd < 0){
	printf("file %s open failed!\r\n", argv[1]);
	return -1;
	}

    int ss = socket(AF_INET, SOCK_STREAM, 0);   //创建套接字，任意IP
    struct sockaddr_in server_sockaddr;     //服务器socket地址结构
    server_sockaddr.sin_family = AF_INET;   //协议族IPv4
    server_sockaddr.sin_port = htons(PORT); //端口
    //printf("%d\n",INADDR_ANY);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);    //任意IP地址
    
    //绑定
    if(bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))==-1) 
    {
        printf("bind failed!\r\n");
        exit(1);
    }
    printf("bind success! \r\n");

    if(listen(ss, QUEUE) == -1) //监听
    {
        printf("listen failed!\r\n");
        //perror("listen");
        exit(1);
    }
 
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    ///成功返回非负描述字，出错返回-1
    int conn = accept(ss, (struct sockaddr*)&client_addr, &length);

    /*没有用来存储accpet返回的套接字的数组，所以只能实现server和单个client双向通信*/
    if( conn < 0 )
    {
        printf("accept failed!\r\n");
        exit(1);
    }
    printf("accept success!\r\n");
    
    while(1)
    {
        /*把可读文件描述符的集合清空*/
        FD_ZERO(&rfds);
        /*把标准输入的文件描述符加入到集合中*/
        FD_SET(0, &rfds);
        maxfd = 0;
        /*把当前连接的文件描述符加入到集合中*/
        FD_SET(conn, &rfds);
        /*找出文件描述符集合中最大的文件描述符*/   
        if(maxfd < conn)
            maxfd = conn;
        /*设置超时时间*/
        tv.tv_sec = 5;//设置倒计时
        tv.tv_usec = 0;
        /*等待聊天*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);   //判读文件描述符的可读状态是否有变化
        if(retval == -1) {
            printf("select出错，客户端程序退出\n");
            break;
        } else if(retval == 0) {
            printf("服务端没有任何输入信息，并且客户端也没有信息到来，waiting...\n");
            continue;
        } else {
            /*客户端发来了消息*/
            if(FD_ISSET(conn,&rfds))    //检测文件描述符conn是否在rfds集合中（文件描述符conn是否有变化）
            {
                char buffer[1024];   
                memset(buffer, 0 ,sizeof(buffer)); 
                int len = recv(conn, buffer, sizeof(buffer), 0);    //接收来自客户端（client）的数据
                if(strcmp(buffer, "exit\n") == 0) break;    //如果接受到的信息是“exit”，退出循环断开连接
                printf("%s", buffer);
                //send(conn, buffer, len , 0);把数据回发给客户端

                databuf[0] = atoi(&buffer[0]); /* 要执行的操作：打开或关闭 */
                /* 向/dev/gpio 文件写入数据 */	
                retvalue = write(fd, databuf, sizeof(databuf));	//调用write函数 //向设备节点写数据
                if(retvalue < 0){
                    printf("GPIO Control Failed!\r\n");
                    close(fd);
                    return -1;
                }
            }
            /*用户输入信息了,开始处理信息并发送*/
            if(FD_ISSET(0, &rfds))  
            {
                char buf[1024];
                fgets(buf, sizeof(buf), stdin);
                //printf("you are send %s", buf);
                send(conn, buf, sizeof(buf), 0);   //向客户端（client）发送数据
            }
        }
    }

    retvalue = close(fd); /* 关闭文件 */
	if(retvalue < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}

    close(conn);
    close(ss);

    printf("EXIT !! \r\n");
    return 0;
}


/***************************/

1.获取驱动节点设备文件，并打开该驱动节点设备
filename = argv[1];
fd = open(filename, O_RDWR); //调用open函数
 
2.创建socket，初始化socket_in地址结构体
int ss = socket(AF_INET, SOCK_STREAM, 0);//创建套接字，任意IP
struct sockaddr_in server_sockaddr;      //服务器socket地址结构
server_sockaddr.sin_family = AF_INET;    //协议族IPv4
server_sockaddr.sin_port = htons(PORT);  //端口
server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //任意IP地址

3.将socket和socket地址结构体进行绑定
bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))

4.监听来自有无网络连接请求
listen(ss, QUEUE)

5.若有网络连接请求，则接受并进行三次握手协议
accept(ss, (struct sockaddr* )&client_addr, &length)

6.开始进行数据的收发，并对设备文件进行操作
send(conn, buf, sizeof(buf), 0);       //向客户端（client）发送数据
recv(conn, buffer, sizeof(buffer), 0); //接收来自客户端（client）的数据
write(fd, databuf, sizeof(databuf));   //调用write函数 //向设备节点写数据

7.结束之后关闭文件断开连接
close();

