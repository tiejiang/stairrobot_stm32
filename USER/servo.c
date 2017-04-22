/**
(TIM3)舵机配置、初始化  (TIM2)速度控制
**/
#include "servo.h"
#include "comunicate.h"
#include "stdio.h"
#include "delay.h"
#include "stm32f10x_usart.h"
#include "print_redirect_function.h"
#include "dance_action_storage.h"

#define BOTTOM_HANDLE_IN_MAINs

//need to config on the detail condition for every servo
#define HEAD_180_ANGLE_MS			500
#define MIDDLE_180_ANGLE_MS		1000
#define BOTTOM_180_ANGLE_MS		1500

#define PWM_ALL_DISTANCE			(PWM_POSITIVE_90 - PWM_NEGATIVE_90)

#define TIMER_PRESCALER				(SystemCoreClock/1000-1);
//#define OC_COUNTER_DEF				3000

#define OC_COUNTER_DEF				3000   //added by 2617
#define OC_COUNTER_DEF_CHANNEL3  1000  //added by 2617

//debug mode 
//#define DEBUG


extern volatile u16 headCurPos,middleCurPos,bottomCurPos;	//current position in unit of servo count
extern volatile u8 middleRotateStart;

volatile u8 headReached = 1;
volatile u8 middleReached = 1;
//bottom reached is handle in main.c
#if !defined(BOTTOM_HANDLE_IN_MAIN)
volatile u8 bottomReached = 1;
u8 bottomHold = 0;
#endif

u8 headTimerStart = 0;
u8 middleTimerStart = 0;
//bottom reached is handle in main.c
#if !defined(BOTTOM_HANDLE_IN_MAIN)
u8 bottomTimerStart = 0;
#endif
u16 TIM_Channel3_Counter;
u16 PWM_INCRSE = 1900;

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)   //输出比较1事件发生
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1); //清除比较中断标志位
		TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
		HServoGo(0);
		headTimerStart = 0;
		headReached =1;
	}
	else if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)  //输出比较2事件发生
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
		TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
		if (middleRotateStart != 1)
			MServoGo(0);
		
		middleTimerStart = 0;
		middleReached =1;
	}
//bottom reached is handle in main.c
#if !defined(BOTTOM_HANDLE_IN_MAIN)
	else if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);
		TIM_ITConfig(TIM2, TIM_IT_CC3, DISABLE);
		bottomTimerStart = 0;
		if (0 == bottomHold)
			BServoGo(0);
		bottomReached =1;
	}
#endif
	//test  
	if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);
		//TIM_ITConfig(TIM2, TIM_IT_CC3, DISABLE);
		
		TIM_Channel3_Counter = TIM_GetCapture3(TIM2);
		if(PWM_INCRSE<3060){
			PWM_INCRSE++;
			TIM_SetCompare1(TIM3, PWM_INCRSE);
		}
			
		TIM_SetCompare3(TIM2, TIM_Channel3_Counter+OC_COUNTER_DEF_CHANNEL3);
	}
}

/*
 * only config
 * PWM is not effect by seting TIM_Pulse as 0
 * oc timer is not effect when interrupt isn't effect 
 */
void servoInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM2, ENABLE);

	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//start vol rising chip
	GPIO_SetBits(GPIOC, GPIO_Pin_14);

	/*GPIOA Configuration: TIM3 channel 1 2 and 3 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

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

  TIM_ARRPreloadConfig(TIM3, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);

  /* ------------------------------------------------------------------
    TIM2 are used for servo speed control ,the main idea of which is 
	to discretize the rotation and use timer to decide to move to the next 
	discretized minStepOnce.
  --------------------------------------------------------------- */
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;   //设置最大范围
  TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; //设置模式为输出比较（比较成功后不产生输出）
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  /* Output Compare Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF;

  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
  TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

  /* Output Compare Timing Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF;
  TIM_OC2Init(TIM2, &TIM_OCInitStructure);
  TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);

  /* Output Compare Timing Mode configuration: Channel3 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF_CHANNEL3;
  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
  TIM_ITConfig(TIM2, TIM_IT_CC3, DISABLE);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);
	
	/* Output Compare Timing Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = OC_COUNTER_DEF_CHANNEL3;
  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
  TIM_ITConfig(TIM2, TIM_IT_CC4, DISABLE);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);

  TIM_Cmd(TIM2, ENABLE);	 /* TIM2 enable counter */
  

	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void powerOffServo(){
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}

void powerOnServo(){
	GPIO_SetBits(GPIOC, GPIO_Pin_14);
}

u16 headGetRunTime(u16 cur, u16 target)
{
	u32 distance;
	if (cur > target)
		distance = cur - target;
	else
		distance = target - cur;

	return distance * HEAD_180_ANGLE_MS / PWM_ALL_DISTANCE;	
}
// 2617
u16 headGetRunTimeSlowly(u16 cur, u16 target)
{
	u32 distance;
	if (cur > target)
		distance = cur - target;
	else
		distance = target - cur;
	return distance * 3000 / PWM_ALL_DISTANCE;		
}

u16 middleGetRunTime(u16 cur, u16 target)
{
	u32 distance;

	if (middleRotateStart == 2)
		return 2500;//5s for poweron schedule

	if (middleRotateStart == 1)
		return 500;//4s default
	
	if (cur > target)
		distance = cur - target;
	else
		distance = target - cur;

	return distance * MIDDLE_180_ANGLE_MS / PWM_ALL_DISTANCE;  //??? ???
}

//bottom reached is handle in main.c
#if !defined(BOTTOM_HANDLE_IN_MAIN)
u16 bottomGetRunTime(u16 cur, u16 target)
{
	u32 distance;
	if (cur > target)
		distance = cur - target;
	else
		distance = target - cur;

	//change to division only once
	return distance * BOTTOM_180_ANGLE_MS / PWM_ALL_DISTANCE;
}
#endif

void headStartTimer(u16 ms_2)
{
	u16 curCounter = TIM_GetCounter(TIM2);
	
	if (65535 - curCounter < ms_2)
	{
		//reset counter
		TIM_SetCounter(TIM2, 0);
		
		if (middleTimerStart)
			TIM_SetCompare2(TIM2, TIM_GetCapture2(TIM2) - curCounter);
		
	//bottom reached is handle in main.c
	#if !defined(BOTTOM_HANDLE_IN_MAIN)
			if (bottomTimerStart)
				TIM_SetCompare3(TIM2, TIM_GetCapture3(TIM2) - curCounter);
	#endif

		TIM_SetCompare1(TIM2, ms_2);
		TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
	}
	else
	{
		TIM_SetCompare1(TIM2, curCounter + ms_2);
		TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
	}
	headTimerStart = 1;
}

void headStopTimer(void)
{
	headTimerStart = 0;
	TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
}

void middleStartTimer(u16 ms_2)
{
	u16 curCounter = TIM_GetCounter(TIM2);	
	if (65535 - curCounter < ms_2)
	{		
		//reset counter 
		TIM_SetCounter(TIM2, 0);
		
		if (headTimerStart)
			TIM_SetCompare1(TIM2, TIM_GetCapture1(TIM2) - curCounter);
		
	//bottom reached is handle in main.c
	#if !defined(BOTTOM_HANDLE_IN_MAIN)
		if (bottomTimerStart)
			TIM_SetCompare3(TIM2, TIM_GetCapture3(TIM2) - curCounter);
	#endif

		TIM_SetCompare2(TIM2, ms_2);
		TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
	}
	else
	{		
		TIM_SetCompare2(TIM2, curCounter + ms_2);  //使得和下一个事件有相同的翻转周期
		TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
	}
	middleTimerStart = 1;
}

void middleStopTimer(void)
{
	middleTimerStart = 0;
	TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
}

//bottom reached is handle in main.c
#if !defined(BOTTOM_HANDLE_IN_MAIN)
void bottomStartTimer(u16 ms_2)
{
	u16 curCounter = TIM_GetCounter(TIM2);
	if (65535 - curCounter < ms_2)
	{
		//reset counter
		TIM_SetCounter(TIM2, 0);
		
		if (headTimerStart)
			TIM_SetCompare1(TIM2, TIM_GetCapture1(TIM2) - curCounter);
		
		if (middleTimerStart)
			TIM_SetCompare2(TIM2, TIM_GetCapture2(TIM2) - curCounter);

		TIM_SetCompare3(TIM2, ms_2);
		TIM_ITConfig(TIM2, TIM_IT_CC3, ENABLE);
	}
	else
	{
		TIM_SetCompare3(TIM2, curCounter + ms_2);
		TIM_ITConfig(TIM2, TIM_IT_CC3, ENABLE);
	}
	bottomTimerStart = 1;
}
#endif

void headServoSpeedGo(u16 PWM,int speed) //speed approm 10
{
	u16 i = 1900;
	// speed is not used now
	headReached = 0;
	headStartTimer(headGetRunTime(headCurPos, PWM));	
	HServoGo(headCurPos=PWM);	
}
// 2617
u16 i = PWM_ANGLE_ZERO;
void headServoSpeedGoSlowly(u16 PWM,int speed) //speed approm 10
{
//	u16 i = PWM_ANGLE_ZERO;
	// speed is not used now
	headReached = 0;
	headStartTimer(headGetRunTimeSlowly(headCurPos, PWM));	
	if(i>PWM){
		while(i>PWM){
			HServoGo(i);
			i--;
			delay_ms(5);
		}
	}else if(i<PWM){
		while(i<PWM){
			HServoGo(i);
			i++;
			delay_ms(5);
		}		
//		i = PWM_ANGLE_ZERO;
	}
//	while(i>1250){
//		HServoGo(headCurPos=i);
//		i--;
//		delay_ms(10);
//	}
	//printf("value_i: %d\n", i);
	headCurPos=PWM;
	//HServoGo(headCurPos=PWM);	
}

u16 j = PWM_ANGLE_ZERO;
void middleServoSpeedGoSlowly(u16 PWM,int speed){
	
	middleReached = 0;
	middleStartTimer(headGetRunTimeSlowly(middleCurPos, PWM));	
	if(j>PWM){
		while(j>PWM){
			MServoGo(j);
			j--;
			delay_ms(5);
		}
	}else if(j<PWM){
		while(j<PWM){
			MServoGo(j);
			j++;
			delay_ms(5);
		}		
//		i = PWM_ANGLE_ZERO;
	}
//	while(i>1250){
//		HServoGo(headCurPos=i);
//		i--;
//		delay_ms(10);
//	}
	//printf("value_i: %d\n", i);
	middleCurPos=PWM;
	//HServoGo(headCurPos=PWM);	
}

void middleServoSpeedGo(u16 PWM,int speed)  //speed approm 10
{
	// speed is not used now
	middleReached = 0;
	middleStartTimer(middleGetRunTime(middleCurPos, PWM));
	MServoGo(middleCurPos=PWM);
}

void middleServoRotate(u16 PWM, u16 ms_2)
{
	middleReached = 0;
	middleStartTimer(ms_2);
	MServoGo(PWM);
}

void middleServoStopRotate(void)
{
	middleStopTimer();
	MServoGo(0);
}

/* 
 * note : always hold the pwm, stop by main process
 */
void bottomServoSpeedGo(u16 PWM,int speed, u8 hold) //speed approm 10
{
	// speed is not used now
//bottom reached is handle in main.c
#if !defined(BOTTOM_HANDLE_IN_MAIN)
	bottomHold = hold;
	bottomReached = 0;
	bottomStartTimer(bottomGetRunTime(bottomCurPos, PWM));
#endif
	BServoGo(bottomCurPos=PWM);
}

