/*******************************************************************************
// GY-521 MPU6050 IIC���Գ���
// ʹ�õ�Ƭ��STM32F103C8T6
// ����8.00M
// ���뻷�� Keil uVision4
// ��ģ������ GPIOB6->SCL GPIOB7->SDA      
// ʹ�ã�STM32F103C8T6����1���ӵ���
// ���Դ���������ʾ�������ʣ�115200
*******************************************************************************/

//#include "stm32f10x_lib.h"
#include <stm32f10x.h>
#include  <math.h>    //Keil library 
#include "mpu6050.h"
#include "usart.h"

//#define   uchar unsigned char
//#define   uint unsigned int	
unsigned char TX_DATA[4];  	 //��ʾ�ݻ�����
unsigned char BUF[10];       //�������ݻ�����
char  test=0; 				 //IIC�õ�
short T_X,T_Y,T_Z,T_T;		 //X,Y,Z�ᣬ�¶�
void Send_data(uchar axis);
void USART1_SendData(uchar SendData);

  /*******************************/
void DATA_printf(uchar *s,short temp_data)
{
	if(temp_data<0){
		temp_data=-temp_data;
    *s='-';
	}
	else *s=' ';
    *++s =temp_data/100+0x30;
    temp_data=temp_data%100;     //ȡ������
    *++s =temp_data/10+0x30;
    temp_data=temp_data%10;      //ȡ������
    *++s =temp_data+0x30; 	
}

void DATA_printf_X_Y_Z(){
	
	DATA_printf(TX_DATA,T_X);//ת��X�����ݵ�����
	Send_data('X');			 //����X����
	DATA_printf(TX_DATA,T_Y);//ת��Y�����ݵ�����
	Send_data('Y');			 //����Y����
	DATA_printf(TX_DATA,T_Z);//ת��Z�����ݵ�����
	Send_data('Z');			 //����Z����		 
//		DATA_printf(TX_DATA,T_T);//ת���¶����ݵ�����
//		Send_data('T');			 //�����¶�����
	USART1_SendData(0X0D);	 //����
	USART1_SendData(0X0A);	 //�س�	
}

 //************************************************
void  USART1_SendData(uchar SendData)
{
	USART_SendData(USART1, SendData);
	Delayms(1);
}

//********���ڷ�������***************************************
void Send_data(uchar axis)
{
	uchar i;
	USART1_SendData(axis);
	USART1_SendData(':');
	for(i=0;i<4;i++)USART1_SendData(TX_DATA[i]);
	USART1_SendData(' ');
	USART1_SendData(' ');
}

/*******************************************************************************
* Function Name  : I2C_GPIO_Config
* Description    : Configration Simulation IIC GPIO
* Input          : None 
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : I2C_delay
* Description    : Simulation IIC Timing series delay
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_delay(void)
{
		
//   u8 i=30; //��������Ż��ٶ�	����������͵�5����д��
	u8 i = 60;
   while(i) 
   { 
     i--; 
   }  
}

void delay5ms(void)
{
		
   int i=5000;  
   while(i) 
   { 
     i--; 
   }  
}
/*******************************************************************************
* Function Name  : I2C_Start
* Description    : Master Start Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : Wheather	 Start
****************************************************************************** */
bool I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA��Ϊ�͵�ƽ������æ,�˳�
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
	SDA_L;
	I2C_delay();
	return TRUE;
}
/*******************************************************************************
* Function Name  : I2C_Stop
* Description    : Master Stop Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
} 
/*******************************************************************************
* Function Name  : I2C_Ack
* Description    : Master Send Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}   
/*******************************************************************************
* Function Name  : I2C_NoAck
* Description    : Master Send No Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
} 
/*******************************************************************************
* Function Name  : I2C_WaitAck
* Description    : Master Reserive Slave Acknowledge Single
* Input          : None
* Output         : None
* Return         : Wheather	 Reserive Slave Acknowledge Single
****************************************************************************** */
bool I2C_WaitAck(void) 	 //����Ϊ:=1��ACK,=0��ACK
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
		SCL_L;
		I2C_delay();
		return FALSE;
	}
	SCL_L;
	I2C_delay();
	return TRUE;
}
/*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : Master Send a Byte to Slave
* Input          : Will Send Date
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_SendByte(u8 SendByte) //���ݴӸ�λ����λ//
{
    u8 i=8;
    while(i--)
    {
        SCL_L;
        I2C_delay();
      if(SendByte&0x80)
        SDA_H;  
      else 
        SDA_L;   
        SendByte<<=1;
        I2C_delay();
		SCL_H;
        I2C_delay();
    }
    SCL_L;
}  
/*******************************************************************************
* Function Name  : I2C_RadeByte
* Description    : Master Reserive a Byte From Slave
* Input          : None
* Output         : None
* Return         : Date From Slave 
****************************************************************************** */
unsigned char I2C_RadeByte(void)  //���ݴӸ�λ����λ//
{ 
    u8 i=8;
    u8 ReceiveByte=0;

    SDA_H;				
    while(i--)
    {
      ReceiveByte<<=1;      
      SCL_L;
      I2C_delay();
	  SCL_H;
      I2C_delay();	
      if(SDA_read)
      {
        ReceiveByte|=0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
} 
//ZRX          
//���ֽ�д��*******************************************

bool Single_Write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_data)		     //void
{
  	if(!I2C_Start())return FALSE;
    I2C_SendByte(SlaveAddress);   //�����豸��ַ+д�ź�//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//���ø���ʼ��ַ+������ַ 
    if(!I2C_WaitAck()){I2C_Stop(); return FALSE;}
    I2C_SendByte(REG_Address );   //���õ���ʼ��ַ      
    I2C_WaitAck();	
    I2C_SendByte(REG_data);
    I2C_WaitAck();   
    I2C_Stop(); 
    delay5ms();
    return TRUE;
}

//���ֽڶ�ȡ*****************************************
unsigned char Single_Read(unsigned char SlaveAddress,unsigned char REG_Address)
{   unsigned char REG_data;     	
	if(!I2C_Start())return FALSE;
    I2C_SendByte(SlaveAddress); //I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//���ø���ʼ��ַ+������ַ 
    if(!I2C_WaitAck()){I2C_Stop();test=1; return FALSE;}
    I2C_SendByte((u8) REG_Address);   //���õ���ʼ��ַ      
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(SlaveAddress+1);
    I2C_WaitAck();

	REG_data= I2C_RadeByte();
    I2C_NoAck();
    I2C_Stop();
    //return TRUE;
	return REG_data;

}


/*
********************************************************************************
** �������� �� GPIO_Configuration(void)
** �������� �� �˿ڳ�ʼ��
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
//void GPIO_Configuration(void)
//{
//  GPIO_InitTypeDef GPIO_InitStructure;
//  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE  );
//   /* Configure USART1 Tx (PA.09) as alternate function push-pull */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //	ѡ�йܽ�9
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		 // �����������
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 // ����������50MHz
//  GPIO_Init(GPIOA, &GPIO_InitStructure);				 // ѡ��A�˿�
//    
//  /* Configure USART1 Rx (PA.10) as input floating */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  //ѡ�йܽ�10
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	  //��������
//  GPIO_Init(GPIOA, &GPIO_InitStructure);				  //ѡ��A�˿�

//}

/*
********************************************************************************
** �������� �� USART1_Configuration(void)
** �������� �� ����1��ʼ��
** ��    ��	�� ��
** ��    ��	�� ��
** ��    ��	�� ��
********************************************************************************
*/
//void USART1_Configuration(void)
//{

//	USART_InitTypeDef USART_InitStructure;
//	USART_ClockInitTypeDef  USART_ClockInitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 |RCC_APB2Periph_USART1, ENABLE  );

//	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;			// ʱ�ӵ͵�ƽ�
//	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;				// ʱ�ӵ͵�ƽ
//	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;				// ʱ�ӵڶ������ؽ������ݲ���
//	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;		// ���һλ���ݵ�ʱ�����岻��SCLK���
//	/* Configure the USART1 synchronous paramters */
//	USART_ClockInit(USART1, &USART_ClockInitStructure);					// ʱ�Ӳ�����ʼ������
//																		 
////	USART_InitStructure.USART_BaudRate = 115200;						  // ������Ϊ��115200
//	USART_InitStructure.USART_BaudRate = 9600;						  // ������Ϊ��9600
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			  // 8λ����
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;				  // ��֡��β����1��ֹͣλ
//	USART_InitStructure.USART_Parity = USART_Parity_No ;				  // ��żʧ��
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// Ӳ��������ʧ��

//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		  // ����ʹ��+����ʹ��
//	/* Configure USART1 basic and asynchronous paramters */
//	USART_Init(USART1, &USART_InitStructure);
//			
//		/* Enable USART1 */
////	USART_ClearFlag(USART1, USART_IT_RXNE); 			//���жϣ�����һ�����жϺ����������ж�
////	USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);		//ʹ��USART1�ж�Դ
////	USART_Cmd(USART1, ENABLE);							//USART1�ܿ��أ����� 
//	
//	#if EN_USART1_RX		  //���ʹ���˽���  
//  //Usart1 NVIC ����
//  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
//	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
//   
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
//#endif
//    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
//}

//��ʼ��MPU6050��������Ҫ��ο�pdf�����޸�************************
void Init_MPU6050(void)
{
/*
   Single_Write(MPU6050_Addr,PWR_M, 0x80);   //
   Single_Write(MPU6050_Addr,SMPL, 0x07);    //
   Single_Write(MPU6050_Addr,DLPF, 0x1E);    //��2000��
   Single_Write(MPU6050_Addr,INT_C, 0x00 );  //
   Single_Write(MPU6050_Addr,PWR_M, 0x00);   //
*/
   	Single_Write(MPU6050_Addr,PWR_MGMT_1, 0x00);	//�������״̬
	Single_Write(MPU6050_Addr,SMPLRT_DIV, 0x07);
	Single_Write(MPU6050_Addr,CONFIG, 0x06);
	Single_Write(MPU6050_Addr,GYRO_CONFIG, 0x18);
	Single_Write(MPU6050_Addr,ACCEL_CONFIG, 0x01);
}
	
//******��ȡMPU6050����****************************************
void READ_MPU6050(void)
{
   BUF[0]=Single_Read(MPU6050_Addr,GYRO_XOUT_L); 
   BUF[1]=Single_Read(MPU6050_Addr,GYRO_XOUT_H);
   T_X=	(BUF[1]<<8)|BUF[0];
   T_X/=16.4; 						   //��ȡ����X������

   BUF[2]=Single_Read(MPU6050_Addr,GYRO_YOUT_L);
   BUF[3]=Single_Read(MPU6050_Addr,GYRO_YOUT_H);
   T_Y=	(BUF[3]<<8)|BUF[2];
   T_Y/=16.4; 						   //��ȡ����Y������
   BUF[4]=Single_Read(MPU6050_Addr,GYRO_ZOUT_L);
   BUF[5]=Single_Read(MPU6050_Addr,GYRO_ZOUT_H);
   T_Z=	(BUF[5]<<8)|BUF[4];
   T_Z/=16.4; 					       //��ȡ����Z������

  // BUF[6]=Single_Read(MPU6050_Addr,TEMP_OUT_L); 
  // BUF[7]=Single_Read(MPU6050_Addr,TEMP_OUT_H); 
  // T_T=(BUF[7]<<8)|BUF[6];
  // T_T = 35+ ((double) (T_T + 13200)) / 280;// ��ȡ������¶�
}
