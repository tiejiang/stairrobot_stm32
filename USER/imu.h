
#ifndef  _IMU_H_
#define _IMU_H_

//Loobot Pet Mode 
#define WAIT 								0   // Loobot pet mode init state
#define START 						  1   // start Looobot pet mode
#define STOP 								2   // stop Loobot pet mode

extern void imuInit(void);
extern void imuUartEnable(void);
extern void imuUartDisable(void);
extern void imuResetZ(void);
#endif

