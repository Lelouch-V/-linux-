使用tcp协议进行通信，服务端进行监听，
全双工，固定回环ip

引脚
/* 
 *  SPI1-CS1  0_3   蜂鸣器
 *  SPI1-CLK  3_7   左轮方向A2
 *  SPI1-SDO  4_0   左轮方向A1

 *  SPI1-SDI  4_1   右轮方向B1
 *  SPI1-CS0  4_2   右轮方向B2

 *  PWM1      7_3	左电机速度
 *  PWM2      7_4	右电机速度
 */


连wifi：
wpa_supplicant -D wext -c /etc/wpa_supplicant.conf -i ra0 &
udhcpc -i ra0
ping -I 192.168.1.126 www.baidu.com


1.make 编译GPIO_ocdev.c成 .ko模块，insmod加载ko模块
2.arm-hisiv300-linux-gcc server.c -o server 编译server.c 为可执行文件，由开发板执行
3. gcc client.c -o client 编译client.c 为可执行文件，由ubuntu执行 
4. 开发板先执行server； ./server /dev/cardrviver
5.ubuntu执行 ./client，输入开发板的ip地址，建立连接

7：蜂鸣器
8：小车前进
4：小车左转
6：小车右转
2：小车后退
5：小车停止
0：退出
输入“exit”则断开socket连接
