/************************<<  ServoControl  >>***********************
  Servo	:  Stop  RotateCw	 -90	  0	       90   RotateCCw      *
-------------------------------------------------------------------*
  PWM 	:   0	     ~	     740  ~	 1900  ~  3060     ~		   *
*******************************************************************/

#ifndef _SERVO_H
#define	_SERVO_H

#include <stm32f10x.h>

//#define PWM_POSITIVE_90 	3060
//#define PWM_NEGATIVE_90 	740
#define PWM_POSITIVE_90 	2600
#define PWM_NEGATIVE_90 	750

#define HServoGo(CCR_Val) TIM_SetCompare1(TIM3,CCR_Val)
#define MServoGo(CCR_Val) TIM_SetCompare2(TIM3,CCR_Val)
#define BServoGo(CCR_Val) TIM_SetCompare3(TIM3,CCR_Val)


void servoInit(void);
void headServoSpeedGo(u16 PWM,int speed);	
void headServoSpeedGoSlowly(u16 PWM,int speed);	
void middleServoSpeedGoSlowly(u16 PWM,int speed);	
void middleServoSpeedGo(u16 PWM,int speed);
void middleServoRotate(u16 PWM, u16 ms);
void middleServoStopRotate(void);
void bottomServoSpeedGo(u16 PWM,int speed, u8 hold);
void headStopTimer(void);
void middleStopTimer(void);
void powerOffServo(void);
void powerOnServo(void);
#endif

