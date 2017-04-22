/**
printf �ض���
**/
#include <stm32f10x.h>
#include "stdio.h"

// ��������
int fputc(int ch, FILE *f)
{
	USART_SendData(USART3, (unsigned char) ch);
	while (!(USART3->SR & USART_FLAG_TXE));
//	while (!(USART3->SR & USART_FLAG_TC));
	return (ch);
}

// ��������
int GetKey (void)  { 
	while (!(USART3->SR & USART_FLAG_RXNE));
	return ((int)(USART3->DR & 0x1FF));
}