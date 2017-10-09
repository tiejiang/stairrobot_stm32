
#include <stm32f10x.h>
//#include <math.h>
#include "stdio.h"
#include "delay.h"
#include "key.h"
#include "LCD5110.h"
//#include "stm32f10x_lib.h"
#include  <math.h>    //Keil library 
#include "mpu6050.h"
#include "moto.h"
#include "sys.h"
#include "usart.h"

#define   uchar unsigned char
#define   uint unsigned int	

//void RCC_Configuration(void);
//void GPIO_Configuration(void);
//void NVIC_Configuration(void);
//void USART1_Configuration(void);
//void WWDG_Configuration(void);
void Delay(u32 nTime);
void Delayms(vu32 m);  
void RCC_Configuration(void);

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
*******************************************************************************/
void RCC_Configuration(void)
{  
	RCC_DeInit(); //RCC system reset(for debug purpose) */
	RCC_HSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	RCC_HCLKConfig(RCC_SYSCLK_Div1); 
	RCC_PCLK2Config(RCC_HCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div4);

	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);//设置 PLL 时钟源及倍频系数 8MHz/2*12=48
	RCC_PLLCmd(ENABLE);//使能或者失能 PLL,如果PLL被用于系统时钟,那么它不能被失能
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){}//等待指定的 RCC 标志位设置成功、PLL初始化成功
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//设置系统时钟（SYSCLK） 设置PLL为系统时钟源
	//等待PLL成功用作于系统时钟的时钟源
	// 0x00：HSI 作为系统时钟
	// 0x04：HSE作为系统时钟
	// 0x08：PLL作为系统时钟
	while(RCC_GetSYSCLKSource() != 0x08){}
   /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF , ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO  , ENABLE);
}

/*
********************************************************************************
** 函数名称 ： Delay(vu32 nCount)
** 函数功能 ： 延时函数
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
 void Delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}

/*
********************************************************************************
** 函数名称 ： void Delayms(vu32 m)
** 函数功能 ： 长延时函数	 m=1,延时1ms
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
 void Delayms(vu32 m)
{
  u32 i;
  
  for(; m != 0; m--)	
       for (i=0; i<50000; i++);
}

  /*
********************************************************************************
** 函数名称 ： main(void)
** 函数功能 ： 主函数
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
int main(void)
{ 
	u32 change = 0;
	u32 flag = 1000;
	u8 t;
	u8 len;	
	u16 times=0;
  RCC_Configuration();		 //配置RCC
//  GPIO_Configuration();		 //配置GPIO
//  USART1_Configuration();	 //配置串口1
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
  I2C_GPIO_Config();		 //配置IIC使用端口
  Delayms(10);				 //延时
  Init_MPU6050();		     //初始化MPU6050
	MotoInit();	


  while(1)
  {
//		READ_MPU6050();	         //读取MPU6050数据
//		DATA_printf_X_Y_Z();
//		Delayms(5);				 //延时
		
//		if(change){
//			MotoAhead();
//			change = 0;
//		}else {
//			MotoBack();
//			change = 1;
//		}
//		
//		while(flag < 19999){
//			flag += 1;
//			front_left_wheel(flag);  //CH1 - PB4
////			Delayms(5);
//		}		
//			Delayms(3000);
//		  flag = 1000;
//		
//		
//			front_left_wheel(10000);  //CH1 - PB4
//			front_right_wheel(15000);  //CH2 - PB5
//			back_left_wheel(17000);  //CH3 - PB0
//			back_right_wheel(18000);	//CH4 - PB1
//			printf("\r\n send the message \r\n\r\n");
		if(USART_RX_STA&0x8000)
		{	
//			printf("\r\n send the message_1 \r\n\r\n");		
			
			len=USART_RX_STA&0x3f;//得到此次接收到的数据长度
			printf("\r\n the message you send is:\r\n\r\n");
			for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
			USART_RX_STA=0;
		}else
		{
//			printf("\r\n send the message_2 \r\n\r\n");		
			
			times++;
//			if(times%5000==0)
//			{
//				printf("\r\n usart test \r\n");
//				printf("tiejiang\r\n\r\n");
//			}
//			if(times%200==0)printf("please input data and click enter \n");  
//			if(times%30==0)LED0=!LED0;//闪烁LED,提示系统正在运行.
//			delay_ms(10);   
		}
  }
}




