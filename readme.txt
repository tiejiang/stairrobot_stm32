������ó�ʼ���ļ���servo.c

ʹ�õ�6050�Ľ��ٶȺͽǶ����� ��û��ʹ�ü��ٶ����ݡ�


1������������ã�
PWM�������  ��ʱ�� TIM3  

/************************<<  ServoControl  >>***********************
  Servo	:  Stop  RotateCw	 -90	  0	       90   RotateCCw      *
-------------------------------------------------------------------*
  PWM 	:   0	     ~	     740  ~	 1900  ~  3060     ~		   *
*******************************************************************/


TIM3  ͨ��

CH1--- PA6   ͷ�����
CH2--- PA7   ���ɶ��
CH3--- PB0   �ײ����

2��TIM2 �ԣ��������ת����ɢ����

TIM2��TIM_IT_CC1��ͷ���������ȽϽ��в���
TIM2��TIM_IT_CC2���в����ɶ������ȽϽ��в���

//3��TIM4 ����6050���ݼ��--����ģʽ


2����imu.c�ļ������ڽ���6050��������ݣ�
UART1 PA9��� PA10����

6050 TX--- PA10   RX --- PA9

��ʼ�����ֶ��������ַ�ʽ��ô������ݣ�1��DMA����ͨ��;2��ͨ��CPU���մ������ݡ�

3������λ����ͨ�Ŵ���

UART3 TX---PB10  RX---PB11


4����Ҫָ�����

CmdTp;// flag for Bluetooth Command Type:
	 0 - Unknown yet,
	 1 - basic Instrution, 
	 2 - defined motion

newCmdType2Value = 

curCmdType2Value =  0

curCmdType2CmdIndex = 1


STAND_BALANCE


balanceMode