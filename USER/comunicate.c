/**
DMA通道2、3 和UART3 的发送和接收数据
**/
#include "stm32f10x_usart.h"
#include "comunicate.h"
#include "stdio.h"
#include "debug.h"

#define BT_UART_USE_DMA

#define BT_BAUD_RATE				115200//9600
#define BT_RX_BUFFER_SIZE		30

//debug
//#define DEBUG

#if defined(BT_UART_USE_DMA)
u8 btCurBufferIndex = 0;
u8 btRxBuffer0[BT_RX_BUFFER_SIZE] = {0};
u8 btRxBuffer1[BT_RX_BUFFER_SIZE] = {0};

//this need
u8 btSendStart = 0;
#else
u8 btRxBuffer0[BT_RX_BUFFER_SIZE] = {0};
#endif

u8 btCurRecvLen = 0;
volatile u8 CmdTp = 0;
volatile u8 newCmdType2Value = 0;
//u32 bt_package = 0;
u32 bt_error = 0;
u32 bt_ok_not_recv = 0;
u32 bt_no_ok_recv = 0;

void USART3_IRQHandler(void)
{
#if defined(BT_UART_USE_DMA)
	u8 *btData;

	int i;
	u8 checkSum = 0;
	
	u8 tempCmd;

	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		//clear interrupt
		USART3->SR;
		USART3->DR;
		DMA_Cmd(DMA1_Channel3,DISABLE);   //关闭接收通道
		btCurRecvLen = BT_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3);//获取数据长度
		if (0 == btCurBufferIndex)
		{
			btData = btRxBuffer0;
			//CNDTR---每次传输数据量
		  DMA1_Channel3->CNDTR = BT_RX_BUFFER_SIZE; //预定义每次传输数据量为BT_RX_BUFFER_SIZE=30
			//CMAR---存放存储器地址
		  DMA1_Channel3->CMAR = (u32)btRxBuffer1;
			btCurBufferIndex = 1;
		}
		else
		{
			btData = btRxBuffer1;
		  DMA1_Channel3->CNDTR = BT_RX_BUFFER_SIZE;
		  DMA1_Channel3->CMAR = (u32)btRxBuffer0;
			btCurBufferIndex = 0;
		}
		
		DMA_Cmd(DMA1_Channel3, ENABLE);  //使能DMA1 通道3   --- 串口3准备接收数据		

		for (i = 0; i < btCurRecvLen - 1; i++) //在btCurRecvLen空间里寻找btData的所有数据---len(tCurRecvLen)>len(btData[])
		{
			checkSum += btData[i];
		}
		
		#ifdef DEBUG			
			printf("\n---COMMUNICATE-data---\n");
			printf("btCurRecvLen: %d\n", btCurRecvLen);
			printf("checkSum: %d\n", checkSum);
			printf("btData-int: %d\n", *btData);
			printf("btData-char: %c\n", *btData);
			printf("btData[0]: %c\n", btData[0]);
			printf("btData[1]: %c\n", btData[1]);
			printf("btData[2]: %c\n", btData[2]);
			printf("btData[btCurRecvLen - 1]: %d\n", btData[btCurRecvLen - 1]);
		
			tempCmd = (char)btData[0];
			debugForCmd(tempCmd);
		#endif
		

		if (checkSum == btData[btCurRecvLen - 1]) 
		{
			//bt_package++;
			switch (btData[0])
			{
			case 0x55:
				CmdTp=2;
				newCmdType2Value = btData[1];
				bluetoothUartSend("OK", 2);
				break;

			case 0xaa:
				CmdTp=1;
				break;

			default:
				bluetoothUartSend("wrong_heading_code", 18);
				bt_error++;
				break;
			}
		}
		else
		{
			bluetoothUartSend("irregular_data", 14);
			bt_error++;
		}
	}
	if(USART_GetITStatus(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//出错处理
		USART_ClearITPendingBit(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE);
#else
	btRxBuffer0[btCurRecvLen]=USART_ReceiveData(USART3);

	switch (btCurRecvLen)				//Check Data header
	{
	case 0: 
		if(btRxBuffer0[0]!='A')
			btCurRecvLen=0;
			return;
	case 1: 
		if(btRxBuffer0[1]!='P')
			btCurRecvLen=0;
			return;
	case 2:
		if(btRxBuffer0[2]!='R')
			btCurRecvLen=0;
			return;
	case 3:
			btCurRecvLen=0;
			return;

	//over the buffer
	case BT_RX_BUFFER_SIZE:
		btCurRecvLen=0;
		return;
		
	default:
		if ((btRxBuffer0[btCurRecvLen]=='\n') && (btRxBuffer0[btCurRecvLen-1]=='\r'))
		{
			btCurRecvLen = 0;
			if(btRxBuffer0[5]==0xAA)
			{
				CmdTp=1;
			}
			else 
				if(btRxBuffer0[4]==0x55)
				{
					newCmdType2Value = btRxBuffer0[5];
					CmdTp=2;
				}
			return;
		}
	}
	
	btCurRecvLen++;
#endif
}

#if defined(BT_UART_USE_DMA)
void DMA1_Channel2_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC2) != RESET)//串口发送通道中断
	{
		btSendStart = 0;
		DMA_ClearITPendingBit(DMA1_IT_TC2);//清中断标志位
	}
}
#endif

void bluetoothInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
#if defined(BT_UART_USE_DMA)
	DMA_InitTypeDef  DMA_InitStructure;
#endif
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure; 

	//clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#if defined(BT_UART_USE_DMA)
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#endif

	/* Configure USART3 Tx (PB.10) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure USART3 Rx (PB.11) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

#if defined(BT_UART_USE_DMA)
	btCurBufferIndex = 0;
	DMA_DeInit(DMA1_Channel3);

	//uart recieve dma
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)btRxBuffer0;
	DMA_InitStructure.DMA_BufferSize = (u16)BT_RX_BUFFER_SIZE;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);//设置外设地址
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	/* Clear DMA global flags */
	DMA_ClearFlag(DMA1_FLAG_GL3);
	/* Enable the DMA USART Rx channel */
	DMA_Cmd(DMA1_Channel3, ENABLE);   //DMA通道3  UART3接收数据
#endif

	//uart
	USART_InitStructure.USART_BaudRate = BT_BAUD_RATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

#if defined(BT_UART_USE_DMA)
	/* idle interrupt for dma received */	
	USART_ITConfig(USART3,USART_IT_TC,DISABLE);
	USART_ITConfig(USART3,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);

	/* DMA receive and send */
	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);
#else 
  USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);  
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
#if defined(BT_UART_USE_DMA)
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);  
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
}

void bluetoothUartEnable()
{
  USART_Cmd(USART3, ENABLE);
}

void bluetoothUartDisable()
{
  USART_Cmd(USART3, DISABLE);
}
//DMA通道2 UART3发送数据
void bluetoothUartSend(u8 *Buffer, u32 Length)
{
#if defined(BT_UART_USE_DMA)
	DMA_InitTypeDef  DMA_InitStructure;

	//please check the detail times for this
	//btSendStart = 1;
	
	DMA_DeInit(DMA1_Channel2);

	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Buffer;
	DMA_InitStructure.DMA_BufferSize = Length;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
	DMA_Init(DMA1_Channel2,&DMA_InitStructure);  
	
	/* Clear DMA global flags */
	DMA_ClearFlag(DMA1_FLAG_GL2);    
	DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);//开DMA发送完成中断
	/* Enable the DMA USART Tx Channel */
	DMA_Cmd(DMA1_Channel2, ENABLE);

	while (btSendStart);
#else
	int i;

	USART_SendData(USART3,*Buffer);
	for(i=1;i<Length;i++)
	{
	   	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
	  	USART_SendData(USART3,Buffer[i]);
	}
#endif
}

