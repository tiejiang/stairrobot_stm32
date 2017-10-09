
#include <stm32f10x.h>
//#include <math.h>
#include "comunicate.h"
#include "audio.h"
#include "stdio.h"
#include "dance_action_storage.h"
#include "print_redirect_function.h"
#include "delay.h"
#include "key.h"
#include "LCD5110.h"
//#include "stm32f10x_lib.h"
#include  <math.h>    //Keil library 
#include "mpu6050.h"
#include "moto.h"
#include "sys.h"

//GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;

#define   uchar unsigned char
#define   uint unsigned int	

/* �������� -----------------------------------------------*/
//void RCC_Configuration(void);
//void GPIO_Configuration(void);
void NVIC_Configuration(void);
//void USART1_Configuration(void);
//void WWDG_Configuration(void);
void Delay(u32 nTime);
void Delayms(vu32 m);  
void RCC_Configuration(void);
/* �������� ----------------------------------------------*/				      

/*
********************************************************************************
** �������� �� RCC_Configuration(void)
** �������� �� ʱ�ӳ�ʼ��
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
//void RCC_Configuration(void)
//{   
//  /* RCC system reset(for debug purpose) */
//  RCC_DeInit();

//  /* Enable HSE */
//  RCC_HSEConfig(RCC_HSE_ON);

//  /* Wait till HSE is ready */
//  HSEStartUpStatus = RCC_WaitForHSEStartUp();

//  if(HSEStartUpStatus == SUCCESS)
//  {
//    /* HCLK = SYSCLK */
//    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
//  
//    /* PCLK2 = HCLK */
//    RCC_PCLK2Config(RCC_HCLK_Div1); 

//    /* PCLK1 = HCLK/2 */
//    RCC_PCLK1Config(RCC_HCLK_Div2);

//    /* Flash 2 wait state */
//    FLASH_SetLatency(FLASH_Latency_2);
//    /* Enable Prefetch Buffer */
//    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

//    /* PLLCLK = 8MHz * 9 = 72 MHz */
//    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

//    /* Enable PLL */ 
//    RCC_PLLCmd(ENABLE);

//    /* Wait till PLL is ready */
//    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
//    {
//    }

//    /* Select PLL as system clock source */
//    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

//    /* Wait till PLL is used as system clock source */
//    while(RCC_GetSYSCLKSource() != 0x08)
//    {
//    }
//  } 
//   /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD , ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF , ENABLE);
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO  , ENABLE);  
//}

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

	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);//���� PLL ʱ��Դ����Ƶϵ�� 8MHz/2*12=48
	RCC_PLLCmd(ENABLE);//ʹ�ܻ���ʧ�� PLL,���PLL������ϵͳʱ��,��ô�����ܱ�ʧ��
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){}//�ȴ�ָ���� RCC ��־λ���óɹ���PLL��ʼ���ɹ�
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//����ϵͳʱ�ӣ�SYSCLK�� ����PLLΪϵͳʱ��Դ
	//�ȴ�PLL�ɹ�������ϵͳʱ�ӵ�ʱ��Դ
	// 0x00��HSI ��Ϊϵͳʱ��
	// 0x04��HSE��Ϊϵͳʱ��
	// 0x08��PLL��Ϊϵͳʱ��
	while(RCC_GetSYSCLKSource() != 0x08){}
   /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF , ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO  , ENABLE);
}

/*
********************************************************************************
** �������� �� NVIC_Configuration(void)
** �������� �� �жϳ�ʼ��
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
//void NVIC_Configuration(void)
//{ 
//  NVIC_InitTypeDef NVIC_InitStructure;  
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 
// 
////  NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQChannel;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_Init(&NVIC_InitStructure);

//}

 /*
********************************************************************************
** �������� �� WWDG_Configuration(void)
** �������� �� ���Ź���ʼ��
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
//void WWDG_Configuration(void)
//{
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);	
//  WWDG_SetPrescaler(WWDG_Prescaler_8);	              //  WWDG clock counter = (PCLK1/4096)/8 = 244 Hz (~4 ms)  
//  WWDG_SetWindowValue(0x41);		                 // Set Window value to 0x41
//  WWDG_Enable(0x50);		       // Enable WWDG and set counter value to 0x7F, WWDG timeout = ~4 ms * 64 = 262 ms 
//  WWDG_ClearFlag();			       // Clear EWI flag
//  WWDG_EnableIT();			       // Enable EW interrupt
//}

/*
********************************************************************************
** �������� �� Delay(vu32 nCount)
** �������� �� ��ʱ����
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
 void Delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}

/*
********************************************************************************
** �������� �� void Delayms(vu32 m)
** �������� �� ����ʱ����	 m=1,��ʱ1ms
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
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
** �������� �� WWDG_IRQHandler(void)
** �������� �� ������ǰ�����ж�
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/ 

//void WWDG_IRQHandler(void)
//{
//  /* Update WWDG counter */
//  WWDG_SetCounter(0x50);
//	
//  /* Clear EWI flag */
//  WWDG_ClearFlag(); 
//}

  /*
********************************************************************************
** �������� �� main(void)
** �������� �� ������
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
int main(void)
{ 
	u32 change = 0;
	u32 flag = 1000;
	
  RCC_Configuration();		 //����RCC
  GPIO_Configuration();		 //����GPIO
  USART1_Configuration();	 //���ô���1
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
  I2C_GPIO_Config();		 //����IICʹ�ö˿�
  Delayms(10);				 //��ʱ
  Init_MPU6050();		     //��ʼ��MPU6050
	MotoInit();	


  while(1)
  {
		READ_MPU6050();	         //��ȡMPU6050����
		DATA_printf_X_Y_Z();
		Delayms(5);				 //��ʱ
		
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
  }
}




