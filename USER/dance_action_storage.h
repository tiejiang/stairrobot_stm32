#ifndef  _DANCEACTIONSTORAGE_H_
#define _DANCEACTIONSTORAGE_H_

#include <stm32f10x.h>

u8 judgeStopRoll(void);	
u16 avoidKnock(void);
u16 resetModeRun(void);
u8 keepAngle(void);
u16 Cmd2Interpret(void);
u8 bodyYesStatus(u16 balanceTimes);
u16 bodyYesModeRun(void);
u8 headNoStatus(u16 balanceTimes);
u16 headNoModeRun(void);
u16 enterSleepRun(void);
u8 frontBackStatus(u16 balanceTimes);
u16 frontBackModeRun(void);
u8 leftRightStatus(u16 balanceTimes);
u16 leftRightModeRun(void);
u8 moveLeftStatus(u16 balanceTimes);
u16 moveLeftModeRun(void);
u8 moveRightStatus(u16 balanceTimes);
u16 moveRightModeRun(void);
u8 moveFrontStatus(u16 balanceTimes);
u16 moveFrontModeRun(void);
u8 moveBackStatus(u16 balanceTimes);
u16 moveBackModeRun(void);
u8 swingCircleStatus(u16 balanceTimes);
u16 swingCircleModeRun(void);
u8 rollLeftRightStatus(u16 balanceTimes);
u16 rollLeftRightModeRun(void);
u8 shakeStatus(u16 balanceTimes);
u16 shakeModeRun(void);
u8 headSlowlyAroundStatus(u16 balanceTimes);
u16 headSlowlyAroundModeRun(void);
u8 feelWrongedStatus(u16 balanceTimes);
u16 feelWrongedModeRun(void);
u8 leftMoveOnceStatus(u16 balanceTimes);
u16 leftMoveOnceModeRun(void);
u8 rightMoveOnceStatus(u16 balanceTimes);
u16 rightMoveOnceModeRun(void);
u8 frontMoveOnceStatus(u16 balanceTimes);
u16 frontMoveOnceModeRun(void);
u8 backMoveOnceStatus(u16 balanceTimes);
u16 backMoveOnceModeRun(void);
u8 spinAroundStatus(u16 balanceTimes);
u16 spinAroundModeRun(void);
u8 headLeftStatus(u16 balanceTimes);
u16 headLeftModeRun(void);
u8 headRightStatus(u16 balanceTimes);
u16 headRightModeRun(void);
u8 bodyLeftStatus(u16 balanceTimes);
u16 bodyLeftModeRun(void);
u8 bodyRightStatus(u16 balanceTimes);
u16 bodyRightModeRun(void);

u8 headLeftSlowlyStatus(u16 balanceTimes);
u16 headLeftSlowlyModeRun(void);
u8 headRightSlowlyStatus(u16 balanceTimes);
u16 headRightSlowlyModeRun(void);

u8 headRotateStop(void);

u8 getCurModeStatus(u16 balanceTimes);

#define IMU_ROLLING_MORMAL_ANGLE		60
#define BLOCK_MAX_TIMES							3

//bottom time for body gesture, modifylater
#define BODY_HALF_BOT_TIMES					60/*95*/
#define BODY_LAYDOWN_BOT_TIMES			120/*160*/

//pwm value for body gesture, modifylater
#define BODY_FRONT_HALF_PWM					1250
#define BODY_BACK_HALF_PWM					2000

#define BODY_FRONT_LAY_DOWN					970
#define BODY_BACK_LAY_DOWN					2250

#define ROTATE_CLOCKWISE_PWM				368
#define BODY_HEAD_RIGHT_SLOWLY			2400

#define BODY_ROTATE_FRONT_SLOWLY_CLOCKWISE_PWM		368  //added by 2617

#define BODY_ROTATE_BACK_SLOWLY_CLOCKWISE_PWM		2500  //added by 2617

#define ROTATE_ANTI_CLOCKWISE_PWM		21802

#define PWM_ANGLE_ZERO							((PWM_POSITIVE_90+PWM_NEGATIVE_90)/2)
#define ONE_ANGLE_VALUE							((PWM_POSITIVE_90 - PWM_NEGATIVE_90) / 180)

//action_mode, modifylater depend on protocol
#define ACTION_RESET_MODE						0xff //复位（站立）
#define ACTION_BODY_YES							0x01 //鞠躬 
#define ACTION_HEAD_NO							0x02 //摇头
#define ACTION_ENTER_SLEEP					0x03 //进入睡眠模式-不接受上位机的指令
#define ACTION_FRONT_BACK_MOVE_ONCE	0x04 //前后摆动
#define ACTION_LEFT_RIGHT_MOVE_ONCE	0x05 //左右摆动
#define ACTION_MOVE_LEFT						0x06 //左滚
#define ACTION_MOVE_RIGHT						0x07 //右滚
#define ACTION_MOVE_FRONT						0x08 //前滚
#define ACTION_MOVE_BACK						0x09 //后滚
#define ACTION_SWING_CIRCLE					0x0a //跳舞-圆舞曲-原地旋转
#define ACTION_NAUGHTY							0x0b //撒娇打滚
#define ACTION_SHAKE								0x0c //抖动
#define ACTION_HEAD_SLOWLY_AROUND   0x0f //头部慢速环视

#define ACTION_LEFT_MOVE_ONCE				0x0d //向左倒地
#define ACTION_RIGHT_MOVE_ONCE  	  0x0e //向右倒地
#define ACTION_FRONT_MOVE_ONCE			0x10 //向前倒地
#define ACTION_BACK_MOVE_ONCE				0x11 //向后倒地
#define ACTION_FEEL_WRONGED	   			0x12 //委屈-身体前弓、摇头
#define ACTION_HEAD_LEFT_SLOWLY			0x13 //头部慢速左转--不回中心位
#define ACTION_HEAD_RIGHT_SLOWLY		0x18 //头部慢速右转--不回中心位
#define ACTION_HEAD_LEFT   	   			0x14 //头部左转
#define ACTION_HEAD_RIGHT  	   			0x15 //头部右转
#define ACTION_BODY_LEFT   	   			0x16 //身体左转
#define ACTION_BODY_RIGHT 	   			0x17 //身体右转

//balance mode
#define STAND_BALANCE								1/*0 1 no need to stand balance*/
#define AVOID_KNOCK_FLOOR_BALANCE		2
#define KEEP_ANGLE_BALANCE					3

#define STAND_MIN_ANGLE							6

//servo min alter angle is 0.3 digree, 12 * 0.3 = 4
#define PWM_MIN_ALTER_VALUE					4

//stand balance mode, threathhold for ajust, modifylater
#define BALANCE_ADJUST_X_MIN_ANGLE	30
#define ROTATE_Z_STOP_ANGLE			    40

#endif
