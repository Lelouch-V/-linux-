
/* 
 *  SPI1-CS1  0_3   蜂鸣器
 *  SPI1-CLK  3_7   左轮方向A2
 *  SPI1-SDO  4_0   左轮方向A1
 *  SPI1-SDI  4_1   右轮方向B1
 *  SPI1-CS0  4_2   右轮方向B2
 *  PWM1      7_3	左电机速度
 *  PWM2 	  7_4	右电机速度
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

 /*
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
*/


/* gpio */
/*  GPIO0_3 SPI1-CS1 8
 *  GPIO3_7 SPI1-CLK 2
 *  GPIO4_0 SPI1-SDO 4
 *  GPIO4_1 SPI1-SDI 6
 *  GPIO4_2 SPI1-CS0 
 *  GPIO7_3 PWM1
 *  GPIO7_4 PWM2
*/
#define MUXCTRL_reg12_ADD (0X200F0030)    //GPIO0_3的复用寄存器
#define MUXCTRL_reg20_ADD (0X200F0050)    //GPIO3_7的复用寄存器
#define MUXCTRL_reg21_ADD (0X200F0054)    //GPIO4_0的复用寄存器
#define MUXCTRL_reg22_ADD (0X200F0058)    //GPIO4_1的复用寄存器
#define MUXCTRL_reg23_ADD (0X200F005C)    //GPIO4_2的复用寄存器

#define GPIO0_DATA_ADD (0X20140020)    //GPIO0的数据寄存器
#define GPIO3_DATA_ADD (0X20170200)    //GPIO3的数据寄存器
#define GPIO4_DATA_ADD (0X2018001C)    //GPIO4的数据寄存器

#define GPIO0_DIR_ADD (0X20140400)    //GPIO0的方向寄存器
#define GPIO3_DIR_ADD (0X20170400)    //GPIO3的方向寄存器
#define GPIO4_DIR_ADD (0X20180400)    //GPIO4的方向寄存器

#define PERI_CRG14_ADD (0x20030038) 	//PWM时钟寄存器
#define PWM1_CFG0_ADD  (0x20130020)		//PWM1周期
#define PWM1_CFG1_ADD  (0x20130024)		//PWM1占空比
#define PWM1_CFG2_ADD  (0x20130028)		//PWM1脉冲个数
#define PWM1_CTRL_ADD  (0x2013002C)		//PWM1控制寄存器

#define PWM2_CFG0_ADD  (0x20130040)		//PWM2周期
#define PWM2_CFG1_ADD  (0x20130044)		//PWM2占空比
#define PWM2_CFG2_ADD  (0x20130048)		//PWM2脉冲个数
#define PWM2_CTRL_ADD  (0x2013004C)		//PWM2控制寄存器



#define REGISTER_LENGTH				4  //寄存器长度

/* @description		: 释放flatform设备模块的时候此函数会执行	
 * @param - dev 	: 要释放的设备 
 * @return 			: 无
 */
static void	car_release(struct device *dev)
{
	printk("car device released!\r\n");	
}


/*  
 * 设备资源信息，也就是car所使用的所有寄存器
 */
static struct resource car_resources[] = {
	[0] = {
		.start 	= MUXCTRL_reg12_ADD,
		.end 	= (MUXCTRL_reg12_ADD + REGISTER_LENGTH - 1),
		.flags 	= IORESOURCE_MEM,
	},	
	[1] = {
		.start	= MUXCTRL_reg20_ADD,
		.end	= (MUXCTRL_reg20_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= MUXCTRL_reg21_ADD,
		.end	= (MUXCTRL_reg21_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.start	= MUXCTRL_reg22_ADD,
		.end	= (MUXCTRL_reg22_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[4] = {
		.start	= MUXCTRL_reg23_ADD,
		.end	= (MUXCTRL_reg23_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[5] = {
		.start	= GPIO0_DATA_ADD,
		.end	= (GPIO0_DATA_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[6] = {
		.start	= GPIO3_DATA_ADD,
		.end	= (GPIO3_DATA_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
    [7] = {
		.start	= GPIO4_DATA_ADD,
		.end	= (GPIO4_DATA_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
    [8] = {
		.start	= GPIO0_DIR_ADD,
		.end	= (GPIO0_DIR_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
    [9] = {
		.start	= GPIO3_DIR_ADD,
		.end	= (GPIO3_DIR_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[10] = {
		.start	= GPIO4_DIR_ADD,
		.end	= (GPIO4_DIR_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
    [11] = {
		.start	= PERI_CRG14_ADD,
		.end	= (PERI_CRG14_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
    [12] = {
		.start	= PWM1_CFG0_ADD,
		.end	= (PWM1_CFG0_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
    [13] = {
		.start	= PWM1_CFG1_ADD,
		.end	= (PWM1_CFG1_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[14] = {
		.start	= PWM1_CFG2_ADD,
		.end	= (PWM1_CFG2_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[15] = {
		.start	= PWM1_CTRL_ADD,
		.end	= (PWM1_CTRL_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[16] = {
		.start	= PWM2_CFG0_ADD,
		.end	= (PWM2_CFG0_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[17] = {
		.start	= PWM2_CFG1_ADD,
		.end	= (PWM2_CFG1_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[18] = {
		.start	= PWM2_CFG2_ADD,
		.end	= (PWM2_CFG2_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
	[19] = {
		.start	= PWM2_CTRL_ADD,
		.end	= (PWM2_CTRL_ADD + REGISTER_LENGTH - 1),
		.flags	= IORESOURCE_MEM,
	},
};



/*
 * platform设备结构体 
 */
static struct platform_device cardevice = {
	.name = "hisi-car", // platform 驱动中的 name 字段也要为“hisi-car”，否则设备和驱动匹配失败。
	.id = -1,
	.dev = {
		.release = &car_release,
	},
	.num_resources = ARRAY_SIZE(car_resources),
	.resource = car_resources,
};
		
/*
 * @description	: 设备模块加载 
 * 通过 platform_device_register 向 Linux 内核注册 cardevice 这个 platform 设备
 * @return 		: 无
 */
static int __init cardevice_init(void)
{
	return platform_device_register(&cardevice);
}

/*
 * @description	: 设备模块注销
 * 设备模块卸载函数，在此函数里面通过 platform_device_unregister 从 Linux内核中删除掉 cardevice 这个 platform 设备。
 * @return 		: 无
 */
static void __exit cardevice_exit(void)
{
	platform_device_unregister(&cardevice);
}

module_init(cardevice_init);
module_exit(cardevice_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lgl");

 

