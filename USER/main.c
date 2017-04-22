
#include <stm32f10x.h>
//#include <math.h>
#include "servo.h"
#include "imu.h"
#include "comunicate.h"
#include "audio.h"
#include "stdio.h"
#include "dance_action_storage.h"
#include "print_redirect_function.h"
#include "delay.h"
#include "key.h"

//DEBUG MODE
//#define DEBUG

///* imu uart */
extern volatile u8 newImuData;
extern s16 gAcc[],angleSpeed[],angle[];
///* bt uart */
extern volatile u8 CmdTp;// flag for Bluetooth Command Type: 0 - Unknown yet,1 - basic Instrution, 2 - defined motion
extern volatile u8 newCmdType2Value;
///* servo */
extern volatile u8 headReached,middleReached;//,bottomReached;//flags indicate reach given pos or not 
/* send OK */
u8 OK_RSP[] = "OK";
////current position in unit of servo count 
volatile u16 headCurPos = 0,middleCurPos = 0,bottomCurPos = 0;
////0: rotate is not started, 1: normal rotated, 2:poweron status
extern volatile u8 middleRotateStart; //added by 2617
volatile u8 needCheckAngle = 0;//added by2617
volatile u8 checkStableStatus; //added by 2617
volatile u8 keepAngleState = 0; //added by 2617
///* mode variant */
u8 curCmdType2Value = 0; //added by 2617
u8 curCmdType2CmdIndex = 0; //added by 2617
u16 key = 0;  //键盘扫描值
//Loobot pet mode
extern u8 Loobot_Pet_Mode ;

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
}

u16 addAngle = PWM_ANGLE_ZERO;
void Key_Action(void)
{
	switch(key)
  	{
	   case 1:
//					power_led_control();
//					headServoSpeedGoSlowly(3000, 100);
//					if(addAngle < 3000){
//						addAngle += 100;
//						headServoSpeedGoSlowly(addAngle, 100);	
//					}else{
//						addAngle = PWM_ANGLE_ZERO;				
//						headServoSpeedGoSlowly(addAngle, 100);	
//					}
//				headServoSpeedGo(PWM_ANGLE_ZERO, 100);
//					headStopTimer();
				break;
	   case 2:
					power_led_control();
//					headServoSpeedGoSlowly(1000, 100);				
					if(addAngle > 800){
						addAngle -= 100;
						headServoSpeedGoSlowly(addAngle, 100);	
					}else{
						addAngle = PWM_ANGLE_ZERO;				
						headServoSpeedGoSlowly(addAngle, 100);	
					}
					key = 0;
//			headServoSpeedGo(PWM_ANGLE_ZERO, 100);		
//			headStopTimer();					
				break;
	   case 3: 
					power_led_control();
//					headServoSpeedGoSlowly(3000, 100);
					if(addAngle < 3000){
						addAngle += 100;
						headServoSpeedGoSlowly(addAngle, 100);	
					}else{
						addAngle = PWM_ANGLE_ZERO;				
						headServoSpeedGoSlowly(addAngle, 100);	
					}
					key = 0;
//				headServoSpeedGo(PWM_ANGLE_ZERO, 100);
//					headStopTimer();
				break;
	   default:
	          break;
	}
}

int main(void)
{
	//s8 reverse=0;
	//u16 targeBottomPwm = 0;
	u16 newBottomPwm = 0;
	//u16 test = 0;
	//s16 zAngle = 0;
	u16 balanceMode = 0;
	u16 blockTimes = 0;//avoid blocking while rotating	
	//u32 balanceTimes = 0;
	extern u32 balanceTimes;  //added by 2617
	// System Clocks Configuration	
	RCC_Configuration();
	//u16 i = 1600;	
//	imuInit();
//	bluetoothInit();
	audioInit(); 
	delay_init();		//added by 2617
//	power_led_control(); //added by 2617
	
	servoInit();

//	headServoSpeedGoSlowly(BODY_FRONT_HALF_PWM, 100);
//	headServoSpeedGoSlowly(3000, 100);
//	
//	middleServoSpeedGoSlowly(3000, 100);


	//reset head & bottom servos first
//	imuUartEnable();   //接6050数据的UART1 使能
	
	//start the receiver from host
//	bluetoothUartEnable();
//	curCmdType2Value = 0;
//	curCmdType2CmdIndex = 1;
	
//	headServoSpeedGo(PWM_ANGLE_ZERO, 100);
//	bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
//	while (1)
//	{
//		newBottomPwm = avoidKnock();
//		if (newBottomPwm != 0)
//			BServoGo(newBottomPwm);
//		if (headReached == 1)
//			break;
//	}
//	BServoGo(0);   //TIM3通道3  PWM=0

	//reset middle servo second
//	middleRotateStart = 2;


//	headServoSpeedGo(PWM_ANGLE_ZERO, 100);
//	middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
	

//	power_led_control(); //added by 2617
//	while (1)
//	{
//		if (middleReached == 1)
//			break;
//	}
//	
//	middleRotateStart = 0;		

	//default mode is stand idle
//	balanceMode = STAND_BALANCE;
	#ifdef DEBUG
	//  HServoGo(BODY_BACK_HALF_PWM); //added by 2617
	//HServoGo(BODY_FRONT_HALF_PWM); //added by 2617
		printf("test--2617 \r\n");
		printf("test--2617 \r\n");
		printf("test end --2617 \r\n");
	#endif	
	
	while(1)
	{		
		key = KEY_Scan();
		Key_Action();		 		
		delay_ms(100);
	}
}

