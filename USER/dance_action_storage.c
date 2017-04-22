/*dance action unit and interpret etc*/
#include "dance_action_storage.h"
#include "imu.h"
#include "servo.h"
#include <math.h>
#include "stdio.h"
#include "delay.h"

//debug mode
//#define DEBUG

/* imu uart */
extern volatile u8 newImuData;
extern s16 gAcc[],angleSpeed[],angle[];

/* bt uart */
extern volatile u8 CmdTp;// flag for Bluetooth Command Type: 0 - Unknown yet,1 - basic Instrution, 2 - defined motion
extern volatile u8 newCmdType2Value;

/* servo */
extern volatile u8 headReached,middleReached;//,bottomReached;//flags indicate reach given pos or not 

//current position in unit of servo count 
extern volatile u16 headCurPos, middleCurPos, bottomCurPos;

//0: rotate is not started, 1: normal rotated, 2:poweron status
volatile u8 middleRotateStart = 0;

// this is used to avoid knock
float subSpeedRate = 0.00013;

//balance mode control, avoid hindration
extern volatile u8 needCheckAngle;

/* mode variant */
extern u8 curCmdType2Value;
extern u8 curCmdType2CmdIndex;

u32 balanceTimes = 0;

//Loobot Pet Mode
extern u8 Loobot_Pet_Mode;

//never used ??
s16 getMiddlePwmByImu()
{
	s16 target = 0;

	if ((angleSpeed[1] > 0) && (angleSpeed[1] > BALANCE_ADJUST_X_MIN_ANGLE))
	{
		return target;
	}
	
	if ((angleSpeed[1] < 0) && (angleSpeed[1] - BALANCE_ADJUST_X_MIN_ANGLE < 0))
	{
		return target;
	}

	return target;
}

s16 getBottomAdjustPwmByImu()
{
	s16 change;
	s16 target = 0;

	change = (u16)(subSpeedRate*angleSpeed[1]*angleSpeed[1]);
	if (change > PWM_MIN_ALTER_VALUE)
	{
		if (angleSpeed[1] > 0)
			target = change;
		else 
			target = -change;
	}

	return target;
}



u8 judgeStopRoll()
{
	static int times = 0;
	//testangle[times]=angle[2];
	times++;
	
	if ((angle[2] < 90) && (angle[2] > IMU_ROLLING_MORMAL_ANGLE))
		return 1;

	if (times >= 400)
	{
		times = 0;
		imuResetZ();
	}
	
	return 0;
}


/*
 * to avoid knock head at floor
 * return: if no imu data, return 0, else return adjust pwm value
 */
u16 avoidKnock(void)
{
	s16 bottomAdjustPwm;
	u16 newBottomPwm = 0;
	
	if (newImuData) 
	{
		newImuData = 0;
		
		bottomAdjustPwm = getBottomAdjustPwmByImu();

		//only sub-speed for big pwm
		if ((bottomCurPos > PWM_ANGLE_ZERO) && (bottomAdjustPwm < 0))
		{
			newBottomPwm = bottomCurPos + bottomAdjustPwm;
			//BServoGo(newBottomPwm);//move call finish
			return newBottomPwm;
		}

		//only sub-speed for small pwm
		if ((bottomCurPos < PWM_ANGLE_ZERO) && (bottomAdjustPwm > 0))
		{
			newBottomPwm = bottomCurPos + bottomAdjustPwm;
			//BServoGo(newBottomPwm);
			return newBottomPwm;
		}

		//sub-speed to middle, pwm is between PWM_POSITIVE_90 and PWM_NEGATIVE_90
		if ((bottomCurPos == PWM_ANGLE_ZERO) && (bottomAdjustPwm != 0))
		{
			newBottomPwm = bottomCurPos + bottomAdjustPwm;

			if (newBottomPwm < PWM_NEGATIVE_90)
				newBottomPwm = PWM_NEGATIVE_90;
			else if (newBottomPwm > PWM_POSITIVE_90)
				newBottomPwm = PWM_POSITIVE_90;

			//BServoGo(newBottomPwm);
			return newBottomPwm;
		}
		//BServoGo(bottomCurPos);
		return bottomCurPos;
	}

	return 0;//no imu data
}


#define STABLE_MIN_ADJUST_ANGLE		1
#define PRE_NUMBER								16
extern volatile u8 checkStableStatus;
extern volatile u8 keepAngleState;
u8 preYAngleNum = 0;
u8 rollStatus = 0;
u16 lastbottomPos;
s16 xAngleKeep = 0;
s16 yAngleKeep = 0;
s16 pre3YAngle[PRE_NUMBER] = {0};


u8 keepAngle(void)
{
	int i;
	s16 xAngleChange = 0;
	s16 yAngleChange = 0;
	u16 newBottomPwm = 0;
//	s16 bottomAdjustPwm;
	
	switch (keepAngleState)
	{
	case 0:
		if (newImuData)
		{
			newImuData = 0;
			
			pre3YAngle[preYAngleNum] = angle[1];
			yAngleKeep+=pre3YAngle[preYAngleNum];
			
			preYAngleNum++;
			if (preYAngleNum == PRE_NUMBER)
			{
				yAngleKeep = yAngleKeep / PRE_NUMBER;
				for (i = 0; i < PRE_NUMBER; i++)
				{
					yAngleChange = abs(pre3YAngle[i] - yAngleKeep);
					if (yAngleChange > STABLE_MIN_ADJUST_ANGLE)
					{
						preYAngleNum = 0;
						yAngleKeep = 0;
						break;
					}
				}
				
				if (preYAngleNum == PRE_NUMBER) {
					keepAngleState = 1;
					xAngleKeep = angle[0];
					preYAngleNum = 0;
					lastbottomPos = bottomCurPos;
				}
			}
		}
		break;

	case 1:
		if (newImuData)
		{
			newBottomPwm = avoidKnock();
		#if 1
			//next angle will be simply setted as angle+angleSpeed*time
			//if (yAngleKeep > 0)
			{
				xAngleChange = angle[0] - xAngleKeep;
				yAngleChange = angle[1] - yAngleKeep;

				//no over 90 for y angle
				if (abs(xAngleChange) < 0x40)
				{
					if (abs(yAngleChange) > 0)
					{
						//if condition happen is normal
						if (abs(yAngleChange) < 20)
							newBottomPwm = newBottomPwm + 10 * yAngleChange;
					}
				}
				else//over 90 angle
				{
					if (abs(yAngleChange) > 0)
					{
						//if condition happen is normal
						if (abs(yAngleChange) < 20)
							newBottomPwm = newBottomPwm - 10 * yAngleChange;
					}
				}
			}
			if (rollStatus == 2)
			{
				/* avoid the pwm is not in control */
				if (newBottomPwm < PWM_NEGATIVE_90)
					newBottomPwm = PWM_NEGATIVE_90;
				else if (newBottomPwm > PWM_ANGLE_ZERO)
					newBottomPwm = PWM_ANGLE_ZERO;				
			}
			else
			{
				/* avoid the pwm is not in control */
				if (newBottomPwm < PWM_NEGATIVE_90)
					newBottomPwm = PWM_NEGATIVE_90;
				else if (newBottomPwm > PWM_POSITIVE_90)
					newBottomPwm = PWM_POSITIVE_90;
			}
			
			BServoGo(newBottomPwm);
			#endif
			newImuData = 0;
		}
		break;
	}

	return keepAngleState;
}

u8 standBalanceState = 0;
u8 standBalanceTimes = 0;
s16 lastAngel = 0;

//  never used ?
void standBalance()
{
	double value = 0;
	u8 xAngleExist=0;
	u8 yAngleExit=0;
	s16 resultAngle=0;
	s16 bottomAdjustPwm = 0;
	u16 newBottomPwm = 0;

	if (newImuData)
	{
		switch (standBalanceState)
		{
		case 0:
			if ((angle[0] > STAND_MIN_ANGLE) || (angle[0] < -STAND_MIN_ANGLE)) //X/Y轴角度安全阈值[-STAND_MIN_ANGLE,STAND_MIN_ANGLE]即[-6,6]
			{
				xAngleExist=1;
			}
			if ((angle[1] > STAND_MIN_ANGLE) || (angle[1] < -STAND_MIN_ANGLE))
			{
				yAngleExit=1;
			}

			if (xAngleExist == 0)
			{
				resultAngle = 0;
			}
			else
			{
				if (yAngleExit == 0)
					resultAngle = 90;
				else
				{
					value = angle[0] * 0.1 / angle[1];
					resultAngle = atan((double)angle[0]/(double)angle[1])*57;//180/3.1416;
				}
			}

			if (((lastAngel - resultAngle) > STAND_MIN_ANGLE ) || ((lastAngel - resultAngle) < -STAND_MIN_ANGLE ))
			{
				lastAngel = resultAngle;
				middleServoSpeedGo(resultAngle * ONE_ANGLE_VALUE + PWM_ANGLE_ZERO, 100);//resultAngle*(2600-750)/180 + (2600+750)/2
				standBalanceState = 1;
			}
			else
			{
				if (yAngleExit == 1)
					standBalanceState = 1;
			}
			break;

		case 1:
			if (middleReached)
			{
				standBalanceTimes++;
				if (standBalanceTimes >= 100)
				{
					standBalanceState = 0;
					standBalanceTimes = 0;
					break;
				}
				
				bottomAdjustPwm = getBottomAdjustPwmByImu();
			
				if (bottomAdjustPwm != 0)
				{
					newBottomPwm = bottomCurPos + bottomAdjustPwm;

					/* avoid the pwm is not in control */
					if (newBottomPwm < PWM_NEGATIVE_90)
						newBottomPwm = PWM_NEGATIVE_90;
					else if (newBottomPwm > PWM_POSITIVE_90)
						newBottomPwm = PWM_POSITIVE_90;

					if (((bottomCurPos > PWM_ANGLE_ZERO) && (newBottomPwm > PWM_ANGLE_ZERO)) || 
						((bottomCurPos < PWM_ANGLE_ZERO) && (newBottomPwm < PWM_ANGLE_ZERO)) ||
						(bottomCurPos == PWM_ANGLE_ZERO))
					{
						BServoGo(newBottomPwm);
					}
				}
				else
				{
					if ((((angle[0] > 0) && (angle[0] < STAND_MIN_ANGLE)) || ((angle[0] < 0) && (angle[0] > -STAND_MIN_ANGLE))) && 
						(((angle[1] > 0) && (angle[1] < STAND_MIN_ANGLE)) || ((angle[1] < 0) && (angle[1] > -STAND_MIN_ANGLE))))
					{
						standBalanceState = 2;
					}
				}
			}
			break;

		case 2:
			middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
			standBalanceState = 0;
			lastAngel = 0;
			break;
		}
		newImuData = 0;
	}
}

void middleRotate(u16 ms_2)
{
	needCheckAngle = 0;
	middleRotateStart = 1;
	middleCurPos = 0;
	imuResetZ();
	middleServoRotate(ROTATE_CLOCKWISE_PWM, ms_2);	
}
//added by 2617	
void middleRotateForBodyRotate(u16 PWM, u16 ms_2)
{
	needCheckAngle = 0;
	middleRotateStart = 1;
	middleCurPos = 0;
	imuResetZ();
	//middleServoRotate(ROTATE_CLOCKWISE_PWM, ms_2);
	middleServoRotate(PWM, ms_2);
}

u8 resetModeStatus(u16 balanceTimes)
{
		//a 0xff cmd, return 2 directly
		if ((CmdTp == 0) && (newCmdType2Value == ACTION_RESET_MODE))
			return 2;
		
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 1;
			break;

		case 2:
			if (headReached && middleReached) return 1;
			break;

		case 3:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 2;
			break;
			
		}

		return 0;
}
u16 resetModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;

	case 2:
		headServoSpeedGo(PWM_ANGLE_ZERO, 100);
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;

	case 3:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}

u8 bodyYesStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (balanceTimes >= BODY_HALF_BOT_TIMES*2) return 2;
			break;
		}

		return 0;
}

u16 bodyYesModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
		break;

	case 2:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}

u8 headNoStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
		case 2:
		case 3:
		case 4:
			if (headReached) return 1;
			break;
			
		case 5:
			if (headReached) return 2;
			break;
		}

		return 0;
}
u16 headNoModeRun(void)
{
	u16 balanceMode = 0;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
	case 3:
		headServoSpeedGo(BODY_FRONT_HALF_PWM, 100);
//		HServoGo(BODY_FRONT_HALF_PWM); //added by 2617
//		headReached = 1;
		break;

	case 2:  //注意case编号 curCmdType2CmdIndex=2时进入此case
	case 4:
		headServoSpeedGo(BODY_BACK_HALF_PWM, 100);
//		HServoGo(BODY_BACK_HALF_PWM); //added by 2617
//		headReached = 1;
		break;

	case 5:
		headServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}

u16 enterSleepRun(void)
{
	u16 balanceMode = 0;

	imuUartDisable();
	//audioPAClose();
	
	return balanceMode;
}

u8 frontBackStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (balanceTimes >= (BODY_HALF_BOT_TIMES)) return 1;
			break;

		case 3:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 2;
			break;
		}

		return 0;
}
u16 frontBackModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
		break;

	case 2:
		bottomServoSpeedGo(BODY_BACK_HALF_PWM, 100, 1);
		break;

	case 3:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}

u8 leftRightStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (middleReached) return 1;
			break;
			
		case 2:
 			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 3:
 			if (balanceTimes >= (BODY_HALF_BOT_TIMES)) return 1;
			break;

		case 4:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;

		case 5:
			if (middleReached) return 2;
			break;
		}

		return 0;
}
u16 leftRightModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		balanceMode = 0;
		middleServoSpeedGo(PWM_NEGATIVE_90, 100);
		break;
		
	case 2:
		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
		break;

	case 3:
		bottomServoSpeedGo(BODY_BACK_HALF_PWM, 100, 1);
		break;

	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;

	case 5:
		balanceMode = 0;
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}

u8 moveLeftStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (middleReached && (angle[2] < ROTATE_Z_STOP_ANGLE) && (angle[2] > -ROTATE_Z_STOP_ANGLE))
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 3:
			if (middleReached) return 1;
			break;

		case 4:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 2;
			break;
		}
		
		return 0;
}
u16 moveLeftModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_LAY_DOWN, 100, 1);
		checkStableStatus = 1;
		break;
		
	case 2:
		rollStatus = 1;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotate(1000);
		break;
		
	case 3:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;

	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 1);
		break;
	}
	
	return balanceMode;
}

u8 moveRightStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (middleReached && (angle[2] < ROTATE_Z_STOP_ANGLE) && (angle[2] > -ROTATE_Z_STOP_ANGLE))
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 3:
			if (middleReached) return 1;
			break;

		case 4:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 2;
			break;
		}
		
		return 0;
}
u16 moveRightModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_BACK_LAY_DOWN, 100, 1);
		checkStableStatus = 1;
		break;
		
	case 2:
		rollStatus = 1;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotate(1000);
		break;
		
	case 3:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;

	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 1);
		break;
	}
	
	return balanceMode;
}

u8 moveFrontStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (middleReached) return 1;
			break;
			
		case 2:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 1;
			break;
			
		case 3:
			if (middleReached && (angle[2] < 60) && (angle[2] > -20))
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 4:
			if (middleReached) return 1;
			break;

		case 5:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 2;
			break;
		}
		
		return 0;
}
u16 moveFrontModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		balanceMode = 0;
		middleServoSpeedGo(PWM_NEGATIVE_90, 100);
		break;

	case 2:
		bottomServoSpeedGo(BODY_BACK_LAY_DOWN, 100, 1);
		break;
		
	case 3:
		rollStatus = 1;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotate(1000);
		break;
		
	case 4:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;

	case 5:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 1);
		break;
	}
	
	return balanceMode;
}

u8 moveBackStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (middleReached) return 1;
			break;
			
		case 2:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 1;
			break;
			
		case 3:
			//if (middleReached && (angle[2] < -20) && (angle[2] > -90))
			if (middleReached && (angle[2] < 60) && (angle[2] > -20))  //modified by 2617
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 4:
			if (middleReached) return 1;
			break;

		case 5:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 2;
			break;
		}
		
		return 0;
}
u16 moveBackModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		balanceMode = 0;
		middleServoSpeedGo(PWM_NEGATIVE_90, 100);
		break;

	case 2:
		bottomServoSpeedGo(BODY_FRONT_LAY_DOWN, 100, 1);
		break;
		
	case 3:
		rollStatus = 1;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotate(1000);
		break;
		
	case 4:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
	
		break;

	case 5:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 1);
		
		//middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}

u8 swingCircleStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (middleReached && (angle[2] < ROTATE_Z_STOP_ANGLE) && (angle[2] > -ROTATE_Z_STOP_ANGLE))
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 3:
 			if (middleReached) return 1;
			break;

		case 4:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 2;
			break;
		}
		
		return 0;
}
u16 swingCircleModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
		checkStableStatus = 1;
		break;
		
	case 2:
		rollStatus = 2;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotate(2000);
		break;
		
	case 3:
		balanceMode = KEEP_ANGLE_BALANCE;
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
		
	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}

u8 rollLeftRightStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 1;
			break;
			
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
 			if (middleReached) return 1;
			break;

		case 7:
			if (balanceTimes >= BODY_LAYDOWN_BOT_TIMES) return 2;
			break;
		}
		
		return 0;
}

u16 rollLeftRightModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_BACK_LAY_DOWN, 100, 1);
		break;
		
	case 2:
	case 4:
		middleServoSpeedGo(PWM_NEGATIVE_90, 100);
		break;
		
	case 3:
	case 5:
		middleServoSpeedGo(PWM_POSITIVE_90, 100);
		break;

	case 6:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;		
		
	case 7:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}

u8 shakeStatus(u16 balanceTimes)
{
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= 4) return 1;
			break;
		
		case 2:
			if (balanceTimes >= 4) return 2;
			break;
		}
		
		return 0;
}

u16 shakeModeRun(void)
{
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_BACK_LAY_DOWN, 100, 1);
		break;
	
	case 2:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
	
		//bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 1);
		break;
	}
	
	return balanceMode;
}
//headSlowlyAroundMode
u8 headSlowlyAroundStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
		case 2:
			if (headReached) return 1;
			break;
			
		case 3:
			if (headReached) return 2;
			break;
		}

		return 0;
} 

u16 headSlowlyAroundModeRun(void){
	u16 balanceMode = 0;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
//	case 3:
		headServoSpeedGoSlowly(BODY_FRONT_HALF_PWM, 100);	
		break;

	case 2:
//	case 4:
		headServoSpeedGoSlowly(BODY_BACK_HALF_PWM, 100);
		break;

	case 3:
		headServoSpeedGoSlowly(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}
//feelWrongedMode
u8 feelWrongedStatus(u16 balanceTimes){
	switch (curCmdType2CmdIndex)
	{
	case 1:
		if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
		break;
		
	case 2:
	case 3:
	case 4:
	case 5:
			if (headReached) return 1;
		break;
	case 6:
		if (balanceTimes >= BODY_HALF_BOT_TIMES)
			if(headReached){
				return 2;
			}		
		break;
	}
	return 0;
} 

u16 feelWrongedModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
		switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
		break;
		
	case 2:
	case 4:
			headServoSpeedGo(BODY_FRONT_HALF_PWM, 100);
		break;
	case 3:
	case 5:
		headServoSpeedGo(BODY_BACK_HALF_PWM, 100);
		break;
	case 6:
		headServoSpeedGo(PWM_ANGLE_ZERO, 100);
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;		
	}
	
	return balanceMode;
}
//leftMoveOnceMode
u8 leftMoveOnceStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (middleReached) return 1;
			break;
			
		case 2:
 			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 3:
// 			if (balanceTimes >= (BODY_HALF_BOT_TIMES)) return 1;
//			break;

		case 4:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;

		case 5:
			if (middleReached) return 2;
			break;
		}

		return 0;
} 
u16 leftMoveOnceModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		balanceMode = 0;
		middleServoSpeedGo(PWM_NEGATIVE_90, 100);
		break;
		
	case 2:
		//bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
		bottomServoSpeedGo(BODY_FRONT_LAY_DOWN, 100, 1);
	
		break;

	case 3:
//		bottomServoSpeedGo(BODY_BACK_HALF_PWM, 100, 1);
//		break;

	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;

	case 5:
		balanceMode = 0;
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}
//rightMoveOnceMode
u8 rightMoveOnceStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (middleReached) return 1;
			break;
			
		case 2:
// 			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
//			break;
			
		case 3:
 			if (balanceTimes >= (BODY_HALF_BOT_TIMES)) return 1;
			break;

		case 4:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;

		case 5:
			if (middleReached) return 2;
			break;
		}

		return 0;
}
u16 rightMoveOnceModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		balanceMode = 0;
		middleServoSpeedGo(PWM_NEGATIVE_90, 100);
		break;
		
	case 2:
//		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
//		break;

	case 3:
		bottomServoSpeedGo(BODY_BACK_LAY_DOWN, 100, 1);
		break;

	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;

	case 5:
		balanceMode = 0;
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}
//frontMoveOnceMode
u8 frontMoveOnceStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 2:
//			if (balanceTimes >= (BODY_HALF_BOT_TIMES)) return 1;
//			break;

		case 3:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 2;
			break;
		}

		return 0;
}
u16 frontMoveOnceModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_LAY_DOWN, 100, 1);
		break;

	case 2:
//		bottomServoSpeedGo(BODY_BACK_HALF_PWM, 100, 1);
//		break;

	case 3:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}
//backMoveOnceMode
u8 backMoveOnceStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
//			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
//			break;
			
		case 2:
			if (balanceTimes >= (BODY_HALF_BOT_TIMES)) return 1;
			break;

		case 3:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 2;
			break;
		}

		return 0;
}
u16 backMoveOnceModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
//		bottomServoSpeedGo(BODY_FRONT_HALF_PWM, 100, 1);
//		break;

	case 2:
		bottomServoSpeedGo(BODY_BACK_LAY_DOWN, 100, 1);
		break;

	case 3:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	}
	
	return balanceMode;
}
//headLeftMode
u8 headLeftStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (headReached) return 1;
			break;
			
		case 2:
			if (headReached) return 2;
			break;
		}

		return 0;
}
u16 headLeftModeRun(void){
	u16 balanceMode = 0;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		headServoSpeedGo(BODY_FRONT_HALF_PWM, 100);
		break;

	case 2:
		headServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}
//headRightMode
u8 headRightStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (headReached) return 1;
			break;
			
		case 2:
			if (headReached) return 2;
			break;
		}

		return 0;
}
u16 headRightModeRun(void){
	u16 balanceMode = 0;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		headServoSpeedGo(BODY_BACK_HALF_PWM, 100);
		break;
	case 2:
		headServoSpeedGo(PWM_ANGLE_ZERO, 100);
		break;
	}
	
	return balanceMode;
}
//bodyLeftMode
u8 bodyLeftStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (middleReached && (angle[2] < ROTATE_Z_STOP_ANGLE) && (angle[2] > -ROTATE_Z_STOP_ANGLE))
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 3:
 			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
		
		case 4:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
		
		case 5:
			if (middleReached) return 1;			
			break;
		
		case 6:
			if (middleReached) return 2;			
			break;
		}
		
		return 0;;
}
u16 bodyLeftModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;
	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_LAY_DOWN, 100, 1);
		checkStableStatus = 1;
		break;
		
	case 2:
		rollStatus = 2;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotateForBodyRotate(BODY_ROTATE_FRONT_SLOWLY_CLOCKWISE_PWM,100);  //旋转时间控制
		break;
		
	case 3:
		//balanceMode = KEEP_ANGLE_BALANCE;		
		break;
		
	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	
	case 5:
		
		break;
		
	case 6:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);		
		break;
	}
	
	return balanceMode;
}
//bodyRightMode
u8 bodyRightStatus(u16 balanceTimes){	
	
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
			
		case 2:
			if (middleReached && (angle[2] < ROTATE_Z_STOP_ANGLE) && (angle[2] > -ROTATE_Z_STOP_ANGLE))
			{
				MServoGo(0);
				return 1;
			}
			break;
			
		case 3:
 			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
		
		case 4:
			if (balanceTimes >= BODY_HALF_BOT_TIMES) return 1;
			break;
		
		case 5:
			if (middleReached) return 1;			
			break;
		
		case 6:
			if (middleReached) return 2;			
			break;
		}
		
		return 0;
}
u16 bodyRightModeRun(void){
	u16 balanceMode = AVOID_KNOCK_FLOOR_BALANCE;	
	switch (curCmdType2CmdIndex)
	{
	case 1:
		bottomServoSpeedGo(BODY_FRONT_LAY_DOWN, 100, 1);
		checkStableStatus = 1;
		break;
		
	case 2:
		rollStatus = 2;
		balanceMode = KEEP_ANGLE_BALANCE;
		middleRotateForBodyRotate(BODY_ROTATE_BACK_SLOWLY_CLOCKWISE_PWM,100);  //旋转时间控制
		break;
		
	case 3:
		//balanceMode = KEEP_ANGLE_BALANCE;		
		break;
		
	case 4:
		bottomServoSpeedGo(PWM_ANGLE_ZERO, 100, 0);
		break;
	
	case 5:
		
		break;
		
	case 6:
		middleServoSpeedGo(PWM_ANGLE_ZERO, 100);		
		break;
	}
	
	return balanceMode;
}
//headLeftSlowlyMode
//used to let head servo rotate stair step
u8 isIncrease = 0;
u16 tempAngle = PWM_ANGLE_ZERO;
u8 headLeftSlowlyStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
		case 1:
			if (headReached) return 1;
			break;
			
		case 2:
			if (headReached) return 2;
			break;
		}

		return 0;
}

u16 headLeftSlowlyModeRun(void){
	u16 balanceMode = 0;

	switch (curCmdType2CmdIndex)
	{
	case 1:
		if(isIncrease){
			tempAngle -= 50; 
			headServoSpeedGo(tempAngle,100);
			isIncrease = 0;
		}
		if(tempAngle < BODY_FRONT_LAY_DOWN){
			tempAngle = PWM_ANGLE_ZERO;
		}		
		
		break;

	case 2:

		break;
	}
	
	return balanceMode;
}

//headRightSlowlyMode
u8 headRightSlowlyStatus(u16 balanceTimes){
		switch (curCmdType2CmdIndex)
		{
			case 1:
				if (headReached) return 1;
				break;
				
			case 2:
				if (headReached) return 2;
				break;
		}

		return 0;
}

u16 headRightSlowlyModeRun(void){
	u16 balanceMode = 0;
	
	switch (curCmdType2CmdIndex)
	{
		case 1:
			if(isIncrease){
				tempAngle += 50;
				headServoSpeedGo(tempAngle,100);
				isIncrease = 0;
			}
			if(tempAngle > BODY_HEAD_RIGHT_SLOWLY){
				tempAngle = PWM_ANGLE_ZERO;
			}
			break;

		case 2:
			break;
	}
	
	return balanceMode;
}

/* 
 * curindex is finished, return 1;
 * all cmds is finished, return 2 directly
 */
u8 getCurModeStatus(u16 balanceTimes)
{
	switch (curCmdType2Value)
	{
		//reset mode
		case ACTION_RESET_MODE:						return resetModeStatus(balanceTimes);
		case ACTION_BODY_YES:							return bodyYesStatus(balanceTimes);
		case ACTION_HEAD_NO:							return headNoStatus(balanceTimes);
		case ACTION_ENTER_SLEEP:					return 2;
		case ACTION_FRONT_BACK_MOVE_ONCE:	return frontBackStatus(balanceTimes);
		case ACTION_LEFT_RIGHT_MOVE_ONCE:	return leftRightStatus(balanceTimes);		
		case ACTION_MOVE_LEFT:						return moveLeftStatus(balanceTimes);
		case ACTION_MOVE_RIGHT:						return moveRightStatus(balanceTimes);
		case ACTION_MOVE_FRONT:						return moveFrontStatus(balanceTimes);
		case ACTION_MOVE_BACK:						return moveBackStatus(balanceTimes);
		case ACTION_SWING_CIRCLE:					return swingCircleStatus(balanceTimes);
		case ACTION_NAUGHTY:							return rollLeftRightStatus(balanceTimes);
		case ACTION_SHAKE:								return shakeStatus(balanceTimes);
		case ACTION_HEAD_SLOWLY_AROUND:		return headSlowlyAroundStatus(balanceTimes);
		case ACTION_FEEL_WRONGED: 				return feelWrongedStatus(balanceTimes);		
		case ACTION_LEFT_MOVE_ONCE:				return leftMoveOnceStatus(balanceTimes);
		case ACTION_RIGHT_MOVE_ONCE:			return rightMoveOnceStatus(balanceTimes);
		case ACTION_FRONT_MOVE_ONCE:			return frontMoveOnceStatus(balanceTimes);
		case ACTION_BACK_MOVE_ONCE:				return backMoveOnceStatus(balanceTimes);
		case ACTION_HEAD_LEFT:						return headLeftStatus(balanceTimes);
		case ACTION_HEAD_RIGHT:						return headRightStatus(balanceTimes);
		case ACTION_BODY_LEFT:						return bodyLeftStatus(balanceTimes);
		case ACTION_BODY_RIGHT: 				  return bodyRightStatus(balanceTimes);	
		case ACTION_HEAD_LEFT_SLOWLY:     return headLeftSlowlyStatus(balanceTimes);
		case ACTION_HEAD_RIGHT_SLOWLY:    return headRightSlowlyStatus(balanceTimes);
	}
	return 0;
}

//翻译上位机传输的指令，并执行对应指令的动作
u16 Cmd2Interpret()	//interpret Bluetooth Cammand of Type 1
{
	u16 balanceMode = 0;
	Loobot_Pet_Mode = STOP;  //2617 when the new Cmd is coming the Loobot Pet Mode must be stop
	powerOnServo();  // enable the servo (bottom servo and middle servo)
	switch (curCmdType2Value)
	{
	case ACTION_RESET_MODE:
		balanceMode = resetModeRun();
		break;
	case ACTION_BODY_YES:
		balanceMode = bodyYesModeRun();
		break;

	case ACTION_HEAD_NO:
		balanceMode = headNoModeRun();
		break;

	case ACTION_ENTER_SLEEP:
		balanceMode = enterSleepRun();
		break;
		
	case ACTION_FRONT_BACK_MOVE_ONCE:
		balanceMode = frontBackModeRun();
		break;

	case ACTION_LEFT_RIGHT_MOVE_ONCE:
		balanceMode = leftRightModeRun();
		break;

	case ACTION_MOVE_LEFT:
		balanceMode = moveLeftModeRun();
		break;

	case ACTION_MOVE_RIGHT:
		balanceMode = moveRightModeRun();
		break;

	case ACTION_MOVE_FRONT:
		balanceMode = moveFrontModeRun();
		break;

	case ACTION_MOVE_BACK:
		balanceMode = moveBackModeRun();
		break;

	case ACTION_SWING_CIRCLE:
		balanceMode = swingCircleModeRun();
		break;

	case ACTION_NAUGHTY:
		balanceMode = rollLeftRightModeRun();
		break;

	case ACTION_SHAKE:
		balanceMode = shakeModeRun();
		break;
	
	case ACTION_HEAD_SLOWLY_AROUND:
		balanceMode = headSlowlyAroundModeRun();
		break;
	
	case ACTION_FEEL_WRONGED:
		balanceMode = feelWrongedModeRun();
		break;
	
	
	case ACTION_LEFT_MOVE_ONCE:
		balanceMode = leftMoveOnceModeRun();
		break;

	case ACTION_RIGHT_MOVE_ONCE:
		balanceMode = rightMoveOnceModeRun();
		break;

	case ACTION_FRONT_MOVE_ONCE:
		balanceMode = frontMoveOnceModeRun();
		break;

	case ACTION_BACK_MOVE_ONCE:
		balanceMode = backMoveOnceModeRun();
		break;

	case ACTION_HEAD_LEFT:
		balanceMode = headLeftModeRun();
		break;

	case ACTION_HEAD_RIGHT:
		balanceMode = headRightModeRun();
		break;
	
	case ACTION_BODY_LEFT:
		balanceMode = bodyLeftModeRun();
		break;
	
	case ACTION_BODY_RIGHT:
		balanceMode = bodyRightModeRun();
		break;
	
	case ACTION_HEAD_LEFT_SLOWLY:
		isIncrease = 1;
		balanceMode = headLeftSlowlyModeRun();		
		break;
	
	case ACTION_HEAD_RIGHT_SLOWLY:
		isIncrease = 1;
		balanceMode = headRightSlowlyModeRun();
		break;	
	}

	return balanceMode;
	
}
