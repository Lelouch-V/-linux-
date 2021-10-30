
//ubuntu 执行
//gcc client.c -o client
// ./client 
// 输入server（开发板）的IP地址
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
 
#define MYPORT  7000
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock_cli;
    fd_set rfds;    //文件描述符集合rfds
    struct timeval tv;
    int retval, maxfd;
    char server_ip[15]; //自定义输入ip地址，将恰好的长度赋值给SERVER_IP
    char *SERVER_IP;    //自定义长度，用于赋值
    int n=0;
    int i=0;
    //输入的参数应该要为1个，0
	if(argc != 1){
	printf("输入的参数数量不对!\r\n");
	return -1;
	}

    printf("Please input the serverip:\n");
    scanf("%s", server_ip);     //输入IP字符串
    // 获取输入的IP字符串长度
	for(i=0;i<sizeof(server_ip);i++){
		if(server_ip[i]=='\0')
			break;
		n++;
	}
    //申请空间
    SERVER_IP = (char *)malloc(sizeof(char) *n);    //返回指针
    //赋值
    for( i=0; i<n;i++){
		SERVER_IP[i]=server_ip[i];
	}
    printf("The IP is: %s \n",SERVER_IP);

    ///定义sockfd
    sock_cli = socket(AF_INET,SOCK_STREAM, 0);  //创建socket，成功则返回非负值，失败则为-1

    ///定义sockaddr_in地址结构体
    struct sockaddr_in servaddr;    
    memset(&servaddr, 0, sizeof(servaddr));  //初始化servaddr内存为0
    servaddr.sin_family = AF_INET;  //协议族 IPv4
    servaddr.sin_port = htons(MYPORT);  ///服务器端口
    //servaddr.sin_addr.s_addr = inet_addr("192.168.10.1");  ///服务器ip，回环地址
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  ///服务器ip
    
       
    //连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error \n");
      //  perror("connect");
        exit(1);
    }
    printf("connect success! \n");

    while(1){
        /*把可读文件描述符的集合清空*/
        FD_ZERO(&rfds);
        /*把标准输入的文件描述符加入到集合中*/
        FD_SET(0, &rfds);
        maxfd = 0;
        /*把当前连接的文件描述符加入到集合中*/
        FD_SET(sock_cli, &rfds);
        /*找出文件描述符集合中最大的文件描述符*/   
        if(maxfd < sock_cli)
            maxfd = sock_cli;
        /*设置超时时间*/
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        /*等待聊天*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv); //判断文件描述符的可读状态是否有变化

        if(retval == -1) {
            printf("select出错，客户端程序退出\n");
            break;
        } else if(retval == 0) {
            printf("客户端没有任何输入信息，并且服务器也没有信息到来，waiting...\n");
            continue;
        } else {
            /*服务器发来了消息*/

            //检测文件描述符sock_cli是否在rfds集合中（文件描述符sock_cli是否有变化）
            if(FD_ISSET(sock_cli,&rfds))            {
                char recvbuf[BUFFER_SIZE];
                int len;

                //接受服务器的消息，返回接收到的字节数
                len = recv(sock_cli, recvbuf, sizeof(recvbuf),0); //接收来自服务器（server）的数据
                printf("%s", recvbuf);
                memset(recvbuf, 0, sizeof(recvbuf));    //初始化recvbuf内存全为0
            }
            /*用户输入信息了,开始处理信息并发送*/
            if(FD_ISSET(0, &rfds))  //检测是否有标准输入在rfds集合中
            {
                char sendbuf[BUFFER_SIZE];
                fgets(sendbuf, sizeof(sendbuf), stdin);     //获取客户端输入的消息
                send(sock_cli, sendbuf, strlen(sendbuf),0); //向服务器发送消息

                if(strcmp(sendbuf,"exit\n")==0)     //判读是否退出socket连接
                    break;
            
                memset(sendbuf, 0, sizeof(sendbuf));    //初始化sendbuf内存全为0
            }
        }
    }
 
    close(sock_cli);
    return 0;
}

1.创建socket
socket(AF_INET,SOCK_STREAM, 0);  //创建socket

2. 初始化sockaddr_in地址结构体
struct sockaddr_in servaddr;    
memset(&servaddr, 0, sizeof(servaddr));         //初始化servaddr内存为0
servaddr.sin_family = AF_INET;                  //协议族 IPv4
servaddr.sin_port = htons(MYPORT);              //服务器端口
servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);//服务器ip

3.向server申请建立连接
connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr))

4. 开始进行数据的收发
recv(sock_cli, recvbuf, sizeof(recvbuf),0) //接收来自服务器（server）的数据
send(sock_cli, sendbuf, strlen(sendbuf),0); //向服务器发送消息

5. 结束之后关闭文件断开连接
close();

