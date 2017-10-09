舵机配置初始化文件：servo.c

使用的6050的角速度和角度数据 ，没有使用加速度数据。


1、舵机驱动配置：
PWM输出配置  定时器 TIM3  

/************************<<  ServoControl  >>***********************
  Servo	:  Stop  RotateCw	 -90	  0	       90   RotateCCw      *
-------------------------------------------------------------------*
  PWM 	:   0	     ~	     740  ~	 1900  ~  3060     ~		   *
*******************************************************************/


TIM3  通道

CH1--- PA6   头部舵机
CH2--- PA7   躯干舵机
CH3--- PB0   底部舵机

2、TIM2 对（舵机）旋转做离散处理

TIM2的TIM_IT_CC1对头部舵机输出比较进行捕获
TIM2的TIM_IT_CC2对中部躯干舵机输出比较进行捕获

//3、TIM4 处理6050数据检测--积分模式


2、（imu.c文件）串口接收6050输出的数据：
UART1 PA9输出 PA10输入

6050 TX--- PA10   RX --- PA9

初始化部分定义了两种方式获得串口数据：1、DMA高速通道;2、通过CPU接收串口数据。

3、与上位机的通信串口

UART3 TX---PB10  RX---PB11


4、重要指令、变量

CmdTp;// flag for Bluetooth Command Type:
	 0 - Unknown yet,
	 1 - basic Instrution, 
	 2 - defined motion

newCmdType2Value = 

curCmdType2Value =  0

curCmdType2CmdIndex = 1


STAND_BALANCE


balanceMode