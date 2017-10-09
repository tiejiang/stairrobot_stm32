#ifndef  _BLUETOOTH_H_
#define _BLUETOOTH_H_

/*********************************************
	Command Format: 'A'+'T'+data+'\r'+'\n'
*********************************************/

#include "stm32f10x_usart.h"

extern void bluetoothInit(void);
extern void bluetoothUartEnable(void);
extern void bluetoothUartDisable(void);
extern void bluetoothUartSend(u8 *Buffer, u32 Length);
#endif

