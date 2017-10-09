#include "moto.h"
#include "stm32f10x_usart.h"
#include <stm32f10x.h>
//#include "stm32f10x_lib.h"


/*
 * only config
 * PWM is not effect by seting TIM_Pulse as 0
 * oc timer is not effect when interrupt isn't effect 
 */
void MotoInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);
	GPIO_AFIODeInit();
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3 ,ENABLE ); //部分重映射，没有此句则在PA6 PA7无输出（未找到原因） 
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//start vol rising chip
	GPIO_SetBits(GPIOC, GPIO_Pin_14);

	/*GPIOA Configuration: TIM3 channel 1 2 and 3 as alternate function push-pull */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 ;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	//设置该引脚为复用输出功能,输出TIM3 CH1 CH2的PWM脉冲波形 GPIOB.4	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

	GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Time base configuration 100 hz */
  TIM_TimeBaseStructure.TIM_Period = 20000-1;//20000 - 1;//23000-1;	 
  TIM_TimeBaseStructure.TIM_Prescaler = 24-1;//24 - 1; //21-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;  //设置模式是PWM
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  /* Pwm Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* Pwm Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;

  TIM_OC2Init(TIM3, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* Pwm Mode configuration: Channel3 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;

  TIM_OC3Init(TIM3, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	
	/* Pwm Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;

  TIM_OC4Init(TIM3, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM3, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);

  /* ------------------------------------------------------------------
    TIM2 are used for servo speed control ,the main idea of which is 
	to discretize the rotation and use timer to decide to move to the next 
	discretized minStepOnce.
  --------------------------------------------------------------- */
  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 65535;   //设置最大范围
//  TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER;
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

//  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

//  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; //设置模式为输出比较（比较成功后不产生输出）
//  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

//  /* Output Compare Timing Mode configuration: Channel1 */
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF;

//  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
//  TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
//  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

//  /* Output Compare Timing Mode configuration: Channel2 */
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF;
//  TIM_OC2Init(TIM2, &TIM_OCInitStructure);
//  TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
//  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);

//  /* Output Compare Timing Mode configuration: Channel3 */
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF_CHANNEL3;
//  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
//  TIM_ITConfig(TIM2, TIM_IT_CC3, DISABLE);
//  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);
//	
//	/* Output Compare Timing Mode configuration: Channel4 */
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF_CHANNEL3;
//  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
//  TIM_ITConfig(TIM2, TIM_IT_CC4, DISABLE);
//  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);

//  TIM_Cmd(TIM2, ENABLE);	 /* TIM2 enable counter */
  

	/* Enable the TIM2 gloabal Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
}

void MotoAhead(){
	
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_6);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
}

void MotoBack(){

	GPIO_SetBits(GPIOA, GPIO_Pin_1);
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_5);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_7);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
}


