#include "stm32f10x.h"
#include "key.h"
#include "delay.h"
//#include "SysTick.h"
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);		 

}
//u16 KEY_Scan(void)
//{	 
//	static u8 key_up=1;//�������ɿ���־	
// 
// 
//	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==0||KEY4==0))
//	{
//		delay_ms(5);//ȥ���� 
//		key_up=0;
//		if(KEY0==0)
//		{
//			 
//			return 1;
//		}
//		else if(KEY1==0)
//		{ 
//	     	
//			return 2;
//		}
//		else if(KEY2==0)
//		{
//	    	
//			return 3;
//		}
//		else if(KEY3==0)
//		{
//	    	
//			return 4;
//		}
//		else if(KEY4==0)
//		{
//	    	
//			return 5;
//		}
//	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1)key_up=1; 	    
// 
// 
//	return 0;// �ް�������
//}
