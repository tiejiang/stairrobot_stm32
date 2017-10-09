
#include "stm32f10x_gpio.h"
#include "audio.h"
#include "delay.h"

//PC_13:power
void audioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	//GPIO_SetBits(GPIOC,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

void power_off(){
	GPIO_ResetBits(GPIOC,GPIO_Pin_15);
}

void power_led_control(){
	
		// test code begin 
//	u16 i ;
//	for (i = 0; i < 10; i ++){
//		 	
//		GPIO_SetBits(GPIOC,GPIO_Pin_13);
//		delay_ms(4);
//		
//		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
//		delay_ms(4);
//	}
	
	// test code end 
	
	
	//close power led	when twinkle three times	
//	GPIO_ResetBits(GPIOC,GPIO_Pin_13);//open led
//	delay_ms(100);
	GPIO_SetBits(GPIOC,GPIO_Pin_13);//close led
	delay_ms(100);
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
//	delay_ms(100);
//	GPIO_SetBits(GPIOC,GPIO_Pin_13);
//	delay_ms(500);
//	
//	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
//	delay_ms(500);
//	GPIO_SetBits(GPIOC,GPIO_Pin_13);
}

