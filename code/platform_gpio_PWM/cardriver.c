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
/***************************************************************
文件名		: cardriver.c
描述	   	: platform驱动
***************************************************************/

#define cardev_CNT		1			/* 设备号长度 	*/
#define cardev_NAME		"car_driver"	/* 设备名字 	*/


static bool beep_flag = false;

//定义寄存器虚拟地址指针
 static void __iomem *MUXCTRL_reg12;     //GPIO0_3的复用寄存器的虚拟地址指针
 static void __iomem *MUXCTRL_reg20;     //GPIO3_7的复用寄存器的虚拟地址指针
 static void __iomem *MUXCTRL_reg21;     //GPIO4_0的复用寄存器的虚拟地址指针
 static void __iomem *MUXCTRL_reg22;     //GPIO4_1的复用寄存器的虚拟地址指针
 static void __iomem *MUXCTRL_reg23;     //GPIO4_2的复用寄存器的虚拟地址指针

 static void __iomem *GPIO0_DATA;   //GPIO0的数据寄存器的虚拟地址指针
 static void __iomem *GPIO3_DATA;   //GPIO3的数据寄存器的虚拟地址指针
 static void __iomem *GPIO4_DATA;   //GPIO4的数据寄存器的虚拟地址指针

 static void __iomem *GPIO0_DIR;    //GPIO0的方向寄存器的虚拟地址指针
 static void __iomem *GPIO3_DIR;    //GPIO3的方向寄存器的虚拟地址指针
 static void __iomem *GPIO4_DIR;    //GPIO4的方向寄存器的虚拟地址指针

 static void __iomem *PERI_CRG14;  	//PWM时钟寄存器
 static void __iomem *PWM1_CFG0; 		//PWM1周期
 static void __iomem *PWM1_CFG1;  		//PWM1占空比
 static void __iomem *PWM1_CFG2;  		//PWM1脉冲个数
 static void __iomem *PWM1_CTRL;  		//PWM1控制寄存器

 static void __iomem *PWM2_CFG0;  		//PWM2周期
 static void __iomem *PWM2_CFG1;  		//PWM2占空比
 static void __iomem *PWM2_CFG2;  		//PWM2脉冲个数
 static void __iomem *PWM2_CTRL;  		//PWM2控制寄存器



/* cardev设备结构体 */
struct cardev_dev{
	dev_t devid;			/* 设备号	*/
	struct cdev cdev;		/* cdev		*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备		*/
	int major;				/* 主设备号	*/		
};
struct cardev_dev cardev; 	/* car设备 */

/*
 * @description		: car方向速度控制
 * @return 			: 无
 */
void gpio_driver_switch(u8 sta)
{
    /* 底层驱动，根据车子的情况修改  */
	 u32 val=0;
	 if(sta==8){
		//前进	 
        //A1 4_0=1; A2 3_7=0 ; PA=1
        //B1 4_1=1; B2 4_2=0 ; PB=1
        writel(0x00, GPIO3_DATA); //gpio3_7 = 0
        writel(0x03, GPIO4_DATA); //gpio4_0 = 1;gpio4_1 = 1;gpio4_2 = 0    
       
        val = readl(GPIO3_DATA); 
        printk("GPIO3_DATA = [%x]  \n",val);
        val = readl(GPIO4_DATA); 
        printk("GPIO4_DATA = [%x]  \n",val);
       
	 }else if(sta==2){
         //后退
        //A1 4_0=0; A2 3_7=1 ; PA=1
        //B1 4_1=0; B2 4_2=1 ; PB=1
        writel(0x80, GPIO3_DATA); //gpio3_7 = 1
        writel(0x04, GPIO4_DATA); //gpio4_0 = 0;gpio4_1 = 0 ;gpio4_2 = 1

        val = readl(GPIO3_DATA); 
        printk("GPIO3_DATA = [%x]  \n",val);
        val = readl(GPIO4_DATA); 
        printk("GPIO4_DATA = [%x]  \n",val);

	 }else if(sta==4){
         //左转
        //A1 4_0=0; A2 3_7=1 ; PA=1
        //B1 4_1=1; B2 4_2=0 ; PB=1
        writel(0x80, GPIO3_DATA); //gpio3_7 = 1
        writel(0x02, GPIO4_DATA); //gpio4_0 = 0;gpio4_1 = 1;gpio4_2 = 0

        val = readl(GPIO3_DATA); 
        printk("GPIO3_DATA = [%x]  \n",val);
        val = readl(GPIO4_DATA); 
        printk("GPIO4_DATA = [%x]  \n",val);

	 }else if(sta==6){
         //右转
		//A1 4_0=1; A2 3_7=0 ; PA=1
        //B1 4_1=0; B2 4_2=1 ; PB=1
        writel(0x00, GPIO3_DATA); //gpio3_7 = 0
        writel(0x05, GPIO4_DATA); //gpio4_0 = 1;gpio4_1 = 0;gpio4_2 = 1

        val = readl(GPIO3_DATA); 
        printk("GPIO3_DATA = [%x]  \n",val);
        val = readl(GPIO4_DATA); 
        printk("GPIO4_DATA = [%x]  \n",val);

	 }else if(sta==5){
		//刹车
        //A1 4_0=0; A2 3_7=0 ; PA=0
        //B1 4_1=0; B2 4_2=0 ; PB=0
        writel(0x00, GPIO4_DATA); //gpio4_0 = 0;gpio4_1 = 0;gpio4_2 = 0
		writel(0x00, GPIO3_DATA); //gpio3_7 = 0

        val = readl(GPIO3_DATA); 
        printk("GPIO3_DATA = [%x]  \n",val);
        val = readl(GPIO4_DATA); 
        printk("GPIO4_DATA = [%x]  \n",val);

	 }else if(sta==7){
         //蜂鸣器
            beep_flag = !beep_flag;
            printk("beep_flag = [%d]  \n",beep_flag);
            if(beep_flag == true){
                val = readl(GPIO0_DATA);
                val |= (1<<3);
                writel(val, GPIO0_DATA); // 0_3 = 1
            }else{
                val = readl(GPIO0_DATA);
                val &= ~(1<<3);
                writel(val, GPIO0_DATA); // 0_3 = 0
            }
            val = readl(GPIO0_DATA); 
            printk("GPIO0_DATA = [%x]  \n",val);
     }
          
     
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int car_open(struct inode *inode, struct file *file)
{
    u32 val = 0; 
    printk("car_driver open .\n");

	//filp->private_data = &cardev; /* 设置私有数据  */

    val = readl(GPIO0_DIR); 	
    val &= ~(1 << 3);   		/* 清除以前的设置 */ 
    val |= (1 << 3);    		/* 设置 GPIO0_3 为输出 */ 
    writel(val, GPIO0_DIR); 

    val = readl(GPIO3_DIR); 	
    val &= ~(1 << 7);   		/* 清除以前的设置 */ 
    val |= (1 << 7);    		/* 设置 GPIO3_7 为输出 */ 
    writel(val, GPIO3_DIR); 

    val = readl(GPIO4_DIR); 	
    val &= ~(0x07);   		/* 清除以前的设置 */ 
    val |= 0x07;    		/* 设置 GPIO4_0 GPIO4_1 GPIO4_2为输出 */ 
    writel(val, GPIO4_DIR); 

    
    val = readl(PERI_CRG14); 	//PWM时钟寄存器 时钟源：3MHz
    val &= ~(0x02);   		
    val |= 0x02;    		
    writel(val, PERI_CRG14); 
    //PWM1
    val = readl(PWM1_CFG0); 	//PWM1 周期200 3M/200=15kHz
    val &= ~(0xC8);   		    //200=0xC8
    val |= 0xC8;    		
    writel(val, PWM1_CFG0); 

    val = readl(PWM1_CFG1); 	//PWM1 占空比 50%
    val &= ~(0x64);   		    //200*0.5=100=0x64
    val |= 0x64;    		
    writel(val, PWM1_CFG1); 

    val = readl(PWM1_CTRL); 	//PWM1 控制寄存器
    val &= ~(0x05);   		    //一直输出方波，使能模块
    val |= 0x05;    		
    writel(val, PWM1_CTRL); 
    //PWM2
    val = readl(PWM2_CFG0); 	//PWM2 周期200 3M/200=15kHz
    val &= ~(0xC8);   		    //200=0xC8
    val |= 0xC8;    		
    writel(val, PWM2_CFG0); 

    val = readl(PWM2_CFG1); 	//PWM2 占空比 50%
    val &= ~(0x64);   		    //200*0.5=100=0x64
    val |= 0x64;    		
    writel(val, PWM2_CFG1); 

    val = readl(PWM2_CTRL); 	//PWM2 控制寄存器
    val &= ~(0x05);   		    //一直输出方波，使能模块
    val |= 0x05;    		
    writel(val, PWM2_CTRL); 

    return 0;
}


/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */



static ssize_t car_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
   int   retvalue; 
   unsigned char databuf[1]; 
   unsigned char gpio_stat; 
   
   retvalue = copy_from_user(databuf, buf, count); 
   if(retvalue < 0) { 
	   printk("kernel write failure!\r\n"); 
	   return -EFAULT; 
   } 
    gpio_stat = databuf[0];        /* 获取状态值  */ 
	if(gpio_stat == 8) {  
        printk("车子前进 \n"); 
		gpio_driver_switch(8); 	  

	} else if(gpio_stat == 2){ 
        printk("车子后退 \n"); 
		gpio_driver_switch(2);   

	} else if(gpio_stat == 4){ 
        printk("车子左转 \n"); 
		gpio_driver_switch(4);   

	} else if(gpio_stat == 6){ 
        printk("车子右转 \n"); 
		gpio_driver_switch(6);   

	} else if(gpio_stat == 5){ 
        printk("车子停止 \n"); 
		gpio_driver_switch(5);    

	} else if(gpio_stat == 7){ 
        printk("蜂鸣器 \n"); 
		gpio_driver_switch(7); 
    }
	return 0;
}

/* 设备操作函数 */
static struct file_operations car_fops = {
	.owner = THIS_MODULE,
	.open = car_open,
	.write = car_write,
};

/*
 * @description		: flatform驱动的probe函数，当驱动与设备匹配以后此函数就会执行
 * @param - dev 	: platform设备
 * @return 			: 0，成功;其他负值,失败
 */
static int car_probe(struct platform_device *dev)
{	
	int i = 0;
	int ressize[20];
	u32 val = 0;
	struct resource *carsource[20];
    beep_flag = false;

	printk("car driver and device has matched!\r\n");
	/* 1、获取资源 */
	for (i = 0; i < 20; i++) {
		carsource[i] = platform_get_resource(dev, IORESOURCE_MEM, i); /* 依次MEM类型资源 */
		if (!carsource[i]) {
			dev_err(&dev->dev, "No MEM resource for always on\n");
			return -ENXIO;
		}
		ressize[i] = resource_size(carsource[i]);	
	}	

	/* 2、初始化car */
	/* 寄存器地址映射 */
	MUXCTRL_reg12 = ioremap(carsource[0]->start, ressize[0]);  //复用寄存器地址映射
    MUXCTRL_reg20 = ioremap(carsource[1]->start, ressize[1]);  //复用寄存器地址映射
    MUXCTRL_reg21 = ioremap(carsource[2]->start, ressize[2]);  //复用寄存器地址映射
    MUXCTRL_reg22 = ioremap(carsource[3]->start, ressize[3]);  //复用寄存器地址映射
    MUXCTRL_reg23 = ioremap(carsource[4]->start, ressize[4]);  //复用寄存器地址映射
    
  	GPIO0_DATA = ioremap(carsource[5]->start, ressize[5]);     //数据寄存器地址映射
    GPIO3_DATA = ioremap(carsource[6]->start, ressize[6]);     //数据寄存器地址映射
    GPIO4_DATA = ioremap(carsource[7]->start, ressize[7]);     //数据寄存器地址映射

	GPIO0_DIR = ioremap(carsource[8]->start, ressize[8]);      //方向寄存器地址映射
    GPIO3_DIR = ioremap(carsource[9]->start, ressize[9]);      //方向寄存器地址映射
    GPIO4_DIR = ioremap(carsource[10]->start, ressize[10]);     //方向寄存器地址映射
   
    PERI_CRG14= ioremap(carsource[11]->start, ressize[11]);  	//PWM时钟寄存器
    PWM1_CFG0= ioremap(carsource[12]->start, ressize[12]);  	//PWM1周期寄存器
    PWM1_CFG1= ioremap(carsource[13]->start, ressize[13]);  	//PWM1占空比寄存器
    PWM1_CFG2= ioremap(carsource[14]->start, ressize[14]);  	//PWM1脉冲个数寄存器
    PWM1_CTRL= ioremap(carsource[15]->start, ressize[15]);  	//PWM1控制寄存器
    
    PWM2_CFG0= ioremap(carsource[16]->start, ressize[16]);  	//PWM2周期寄存器
    PWM2_CFG1= ioremap(carsource[17]->start, ressize[17]);      //PWM2占空比寄存器
    PWM2_CFG2= ioremap(carsource[18]->start, ressize[18]);  	//PWM2脉冲个数寄存器
    PWM2_CTRL= ioremap(carsource[19]->start, ressize[19]);  	//PWM2控制寄存器
    
	
	/* 注册字符设备驱动 */
	/*1、创建设备号 */
	if (cardev.major) {		/*  定义了设备号 */
		cardev.devid = MKDEV(cardev.major, 0);
		register_chrdev_region(cardev.devid, cardev_CNT, cardev_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&cardev.devid, 0, cardev_CNT, cardev_NAME);	/* 申请设备号 */
		cardev.major = MAJOR(cardev.devid);	/* 获取分配号的主设备号 */
	}
	
	/* 2、初始化cdev */
	cardev.cdev.owner = THIS_MODULE;
	cdev_init(&cardev.cdev, &car_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&cardev.cdev, cardev.devid, cardev_CNT);

	/* 4、创建类 */
	cardev.class = class_create(THIS_MODULE, cardev_NAME);
	if (IS_ERR(cardev.class)) {
		return PTR_ERR(cardev.class);
	}

	/* 5、创建设备 */
	cardev.device = device_create(cardev.class, NULL, cardev.devid, NULL, cardev_NAME);
	if (IS_ERR(cardev.device)) {
		return PTR_ERR(cardev.device);
	}

	//major = register_chrdev(0, "gpio_drv", &gpio_drv_fops); // 注册, 告诉内核，返回值major为自动分配的主设备号
    // gpio_drv_class = class_create(THIS_MODULE, "gpiodrv");  ///* 创建类 */ 
    // device_create(gpio_drv_class, NULL, MKDEV(major, 0), NULL, DEV_NAME); /* insmod xxx后，会自动生成/dev/xyz设备，并且自动给该设备分配主设备号major */

	return 0;
}

/*
 * @description		: platform驱动的remove函数，移除platform驱动的时候此函数会执行
 * @param - dev 	: platform设备
 * @return 			: 0，成功;其他负值,失败
 */
static int car_remove(struct platform_device *dev)
{
	printk("gpio_driver module exit.\n");

	 /* 删除映射关系 */
    iounmap(MUXCTRL_reg12); 
    iounmap(MUXCTRL_reg20); 
    iounmap(MUXCTRL_reg21); 
    iounmap(MUXCTRL_reg22); 
    iounmap(MUXCTRL_reg23); 

    iounmap(GPIO0_DATA); 
    iounmap(GPIO3_DATA); 
    iounmap(GPIO4_DATA); 

    iounmap(GPIO0_DIR); 
    iounmap(GPIO3_DIR); 
    iounmap(GPIO4_DIR); 

    iounmap(PERI_CRG14); 
    iounmap(PWM1_CFG0); 
    iounmap(PWM1_CFG1); 
    iounmap(PWM1_CFG2); 
    iounmap(PWM1_CTRL); 

    iounmap(PWM2_CFG0); 
    iounmap(PWM2_CFG1); 
    iounmap(PWM2_CFG2); 
    iounmap(PWM2_CTRL); 


	cdev_del(&cardev.cdev);/*  删除cdev */
	unregister_chrdev_region(cardev.devid, cardev_CNT); /* 注销设备号 */
	device_destroy(cardev.class, cardev.devid);
	class_destroy(cardev.class);

	   
    //  删除 cdev 字符设备
    //unregister_chrdev(major, "gpio_drv"); /* 卸载驱动程序，告诉内核 */
    //删除设备 （与 device_create 对应）
    //device_destroy(gpio_drv_class, MKDEV(major, 0));
    //删除类   （与 class_create 对应）
    //class_destroy(gpio_drv_class);

	return 0;
}

/* platform驱动结构体 */
static struct platform_driver car_driver = {
	.driver		= {
		.name	= "hisi-car",			/* 驱动名字，用于和设备匹配 */
	},
	.probe		= car_probe,	//car初始化
	.remove		= car_remove, 	//car卸载
};
		
/*
 * @description	: 驱动模块加载函数
 * 通过 platform_driver_register 向 Linux 内核注册 car_driver 驱动。
 * @return 		: 无
 */
static int __init cardriver_init(void)
{
	return platform_driver_register(&car_driver);
}

/*
 * @description	: 驱动模块卸载函数
 * 通过 platform_driver_unregister 从 Linux内核卸载 car_driver 驱动。
 * @return 		: 无
 */
static void __exit cardriver_exit(void)
{
	platform_driver_unregister(&car_driver);
}

module_init(cardriver_init);
module_exit(cardriver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lgl");



