/**
DMA通道4、5和UART1的数据发送和接收
**/
#include <stm32f10x.h>
#include <stm32f10x_usart.h>

#include <stm32f10x_dma.h>
#include "imu.h"
#include "comunicate.h"
#include "stdio.h"


#define IMU_UART_USE_DMA

//#define IMU_BAUD_RATE				9600
#define IMU_BAUD_RATE				115200
#define IMU_RX_BUFFER_SIZE	44
#define IMU_RX_PROTOCOL_LEN	11

//debug mode
//#define DEBUG

#if defined(IMU_UART_USE_DMA)
u8 imuCurBufferIndex = 0;
u8 imuRxBuffer0[IMU_RX_BUFFER_SIZE] = {0};
u8 imuRxBuffer1[IMU_RX_BUFFER_SIZE] = {0};
#else
u8 imuCurRecvCounter = 0;
u8 imuRxBuffer0[IMU_RX_PROTOCOL_LEN] = {0};
#endif

volatile u8 newImuData = 0;
#if 0//defined(IMU_UART_USE_DMA)
volatile u8 zResetStart = 0;
#endif
s16 gAcc[3] = {0};
s16 angleSpeed[3] = {0};
s16 angle[3] = {0};
s16 tempture = 0;

u32 imu_package = 0;
u32 imu_error = 0;
//value compare for Loobot petMode
u32 temp_x_angle;
u32 temp_y_angle;
u32 temp_z_angle;
u8 Loobot_Pet_Mode = START;
u8 Loobot_Pet_Mode_Rewards = 0;

void USART1_IRQHandler(void)        
{
	s16 temp;
	u8 imuRecvBufferLen = 0;
	u8 *imuData;
	
#if defined(IMU_UART_USE_DMA)
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		//clear interrupt
		USART1->SR;
		USART1->DR;
		DMA_Cmd(DMA1_Channel5,DISABLE);
		imuRecvBufferLen = IMU_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
		//交替进入两个不同的缓存 ：imuRxBuffer0  和  imuRxBuffer1
		if (0 == imuCurBufferIndex)
		{
			imuData = imuRxBuffer0;
		  DMA1_Channel5->CNDTR = IMU_RX_BUFFER_SIZE; //传输数据量
		  DMA1_Channel5->CMAR = (u32)imuRxBuffer1; //存储器
			imuCurBufferIndex = 1;
		}
		else
		{
			imuData = imuRxBuffer1;
		  DMA1_Channel5->CNDTR = IMU_RX_BUFFER_SIZE;  //传输数据量
		  DMA1_Channel5->CMAR = (u32)imuRxBuffer0;		//存储器
			imuCurBufferIndex = 0;
		}
		DMA_Cmd(DMA1_Channel5, ENABLE);
		
		#ifdef DEBUG
			//printf("IMU data received\n"); 
		#endif
		
		//判断是否拿到6050的数据包 ---适用于旧版本6050数据解析
		if ((33 == imuRecvBufferLen) && 
				(0x55 == *(imuData+0)) && (0x51 == *(imuData+1)) && 
				(0x55 == *(imuData+11)) && (0x52 == *(imuData+12)) && 
				(0x55 == *(imuData+22)) && (0x53 == *(imuData+23)))
		{
			imu_package++;
			newImuData = 1;
			//gAcc[0] = (*(imuData + 3) << 8) + *(imuData + 2);
			//gAcc[1] = (*(imuData + 5) << 8) + *(imuData + 4);
			//gAcc[2] = (*(imuData + 7) << 8) + *(imuData + 6);
			angleSpeed[0] = (*(imuData + 14) << 8) + *(imuData + 13);
			angleSpeed[1] = (*(imuData + 16) << 8) + *(imuData + 15);
			angleSpeed[2] = (*(imuData + 18) << 8) + *(imuData + 17); 
			temp = (*(imuData + 25) << 8) + *(imuData + 24);
			angle[0] = temp / 182;
			temp = (*(imuData + 27) << 8) + *(imuData + 26);
			angle[1] = temp / 182;
			temp = (*(imuData + 29) << 8) + *(imuData + 28);
			angle[2] = temp / 182;
			
			if(Loobot_Pet_Mode==START){
					//printf("abs(temp_x_angle-angle[0]): %d\n", abs(temp_x_angle-angle[0]));
					if(abs(temp_x_angle - gAcc[0])>15 || abs(temp_y_angle - gAcc[1])>15){        //	
						//printf("knock!!! \n");
						Loobot_Pet_Mode_Rewards ++;
						if(Loobot_Pet_Mode_Rewards == 10){   //modify 30 at user version
								//printf("receive rewards! \n");
								bluetoothUartSend("JF", 2);
								//bluetoothUartSend(abs(temp_x_angle-angle[0])+0x30, 10);
								Loobot_Pet_Mode_Rewards = 0;
						}
					}
			}

			tempture = (*(imuData + 31) << 8) + *(imuData + 30); 
			temp_x_angle = gAcc[0];
			temp_y_angle = gAcc[1];
			temp_z_angle = gAcc[2]; 
		}

		// 22 为加速度、角速度、角度数据包总长度 ---适用于新版本6050数据解析
//			if ((22 == imuRecvBufferLen) && ('S' == *(imuData+0)) && 'E' == *(imuData + 21))
//		{
//			imu_package++;
//			newImuData = 1;
//			angleSpeed[0] = (*(imuData + 2) << 8) + *(imuData + 1);   //组合X轴高低字节
//			angleSpeed[1] = (*(imuData + 4) << 8) + *(imuData + 3);   //Y~
//			angleSpeed[2] = (*(imuData + 6) << 8) + *(imuData + 5);     //Z~
//			gAcc[0] = (*(imuData + 8) << 8) + *(imuData + 7);
//			gAcc[0] = gAcc[0]/30;
//			gAcc[1] = (*(imuData + 10) << 8) + *(imuData + 9);
//			gAcc[1] = gAcc[1]/60;
//			gAcc[2] = (*(imuData + 12) << 8) + *(imuData + 11);
//			temp = (*(imuData + 14) << 8) + *(imuData + 13);
//			angle[0] = temp / 182;			
//			temp = (*(imuData + 16) << 8) + *(imuData + 15);
//			angle[1] = temp / 182;			
//			temp = (*(imuData + 18) << 8) + *(imuData + 17);
//			angle[2] = temp / 182;			
//			
////		#ifdef DEBUG			
////			printf("Start_Loobot_Pet_Mode: %d\n", Loobot_Pet_Mode);
////		//	printf("balanceMode: %d\n", balanceMode);
////		#endif
//			if(Loobot_Pet_Mode==START){
//					//printf("abs(temp_x_angle-angle[0]): %d\n", abs(temp_x_angle-angle[0]));
//					if(abs(temp_x_angle - gAcc[0])>15 || abs(temp_y_angle - gAcc[1])>15){        //	
//						//printf("knock!!! \n");
//						Loobot_Pet_Mode_Rewards ++;
//						if(Loobot_Pet_Mode_Rewards == 10){   //modify 30 at user version
//								//printf("receive rewards! \n");
//								bluetoothUartSend("JF", 2);
//								//bluetoothUartSend(abs(temp_x_angle-angle[0])+0x30, 10);
//								Loobot_Pet_Mode_Rewards = 0;
//						}
//					}
//			}

//			tempture = (*(imuData + 31) << 8) + *(imuData + 30); 
//			temp_x_angle = gAcc[0];
//			temp_y_angle = gAcc[1];
//			temp_z_angle = gAcc[2];
//		}
		else
			imu_error++;
	}
	if(USART_GetITStatus(USART1, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//出错处理
		USART_ClearITPendingBit(USART1, USART_IT_PE | USART_IT_FE | USART_IT_NE);
#else
	imuRxBuffer0[imuCurRecvCounter]=USART_ReceiveData(USART1);//不同单片机略有差异
	if((imuCurRecvCounter==0)&&(imuRxBuffer0[0]!=0x55)) 
		return; //第 0 号数据不是帧头，跳过

	imuCurRecvCounter++;
	if (IMU_RX_PROTOCOL_LEN == imuCurRecvCounter) //接收到 11 个数据
	{
		switch (imuRxBuffer0[1])
		{
		//case 0x51:      //acceleration
			//gAcc[0] = ((short)(imuRxBuffer0 [3]<<8| imuRxBuffer0 [2]));///32768.0*16;//x
			//gAcc[1] = ((short)(imuRxBuffer0 [5]<<8| imuRxBuffer0 [4]))/32768.0*16;//y
			//gAcc[2] = ((short)(imuRxBuffer0 [7]<<8| imuRxBuffer0 [6]))/32768.0*16;//z
			//break;
		case 0x52:
		  newImuData=1;
		  angleSpeed[0] = ((short)(imuRxBuffer0 [3]<<8| imuRxBuffer0 [2]));///32768.0*2000;	修改取w[1]
			angleSpeed[1] = ((short)(imuRxBuffer0 [5]<<8| imuRxBuffer0 [4]));///32768.0*2000;
			//angleSpeed[2] = ((short)(imuRxBuffer0 [7]<<8| imuRxBuffer0 [6]));///32768.0*2000;
			break;

		case 0x53:     //angle
			//angle[0] = ((short)(imuRxBuffer0 [3]<<8| imuRxBuffer0 [2]));//*45/8192.0;
			angle[1] = ((short)(imuRxBuffer0 [5]<<8| imuRxBuffer0 [4]));//*180/32768.0;
			//angle[2] = ((short)(imuRxBuffer0 [7]<<8| imuRxBuffer0 [6]))*180/32768.0;
			break;
		}

		imuCurRecvCounter=0; //重新赋值，准备下一帧数据的接收
	}
#endif
}

#if 0//defined(IMU_UART_USE_DMA)
void DMA1_Channel4_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC4) != RESET)//串口发送通道中断
	{
		zResetStart = 0;
		DMA_ClearITPendingBit(DMA1_IT_TC4);//清中断标志位
		DMA_Cmd(DMA1_Channel4, DISABLE);
	}
}

void imuUartSend(u8 *Buffer, u32 Length)
{
	DMA_InitTypeDef  DMA_InitStructure; 
	
	DMA_DeInit(DMA1_Channel4);

	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Buffer;
	DMA_InitStructure.DMA_BufferSize = Length;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
	DMA_Init(DMA1_Channel4,&DMA_InitStructure);

	
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	
	/* Clear DMA global flags */
	DMA_ClearFlag(DMA1_FLAG_GL4);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);//开DMA发送完成中断
	/* Enable the DMA USART Tx Channel */
	DMA_Cmd(DMA1_Channel4, ENABLE);
}
#endif

void imuInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure; 

	//clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#if defined(IMU_UART_USE_DMA)
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#endif

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#if defined(IMU_UART_USE_DMA)
	imuCurBufferIndex = 0;
	DMA_DeInit(DMA1_Channel5);

	//uart recieve dma
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)imuRxBuffer0;
	DMA_InitStructure.DMA_BufferSize = (u16)IMU_RX_BUFFER_SIZE;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);//设置外设地址
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	/* Clear DMA global flags */
	DMA_ClearFlag(DMA1_FLAG_GL5);
	/* Enable the DMA USART Rx channel */
	DMA_Cmd(DMA1_Channel5, ENABLE);
#endif

	//uart
	USART_InitStructure.USART_BaudRate = IMU_BAUD_RATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);/* Configure the USART1 */

#if defined(IMU_UART_USE_DMA)
	/* idle interrupt for dma received */	
	USART_ITConfig(USART1,USART_IT_TC,DISABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);

	/* DMA receive and send */
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
#else 
  USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);  
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
#if 0//defined(IMU_UART_USE_DMA)
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);  
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
}

void imuUartEnable(void)
{
	USART_Cmd(USART1, ENABLE);
}

void imuUartDisable(void)
{
	USART_Cmd(USART1, DISABLE);
}

void imuResetZ(void)
{
#if 0//defined(IMU_UART_USE_DMA)
	u8 buffer[] = {0xff, 0xaa, 0x52};

	zResetStart = 1;
	imuUartSend(buffer, 3);
#else
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	USART_SendData(USART1, 0xff);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	USART_SendData(USART1, 0xaa);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	USART_SendData(USART1, 0x52);
#endif
}

