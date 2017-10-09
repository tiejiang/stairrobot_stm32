#ifndef _LCD5110_H
#define _LCD5110_H
#include "sys.h"

//宽高像素
#define LCD_X_RES		84
#define LCD_Y_RES		48

//枚举 D/C模式选择 
typedef enum
{
  DC_CMD  = 0,	//写命令
	DC_DATA = 1		//写数据		
} DCType;

//------------------------------------------------------------移植修改区-----------------------------------------------------------------------

#define LCD_CTRL_PORT	GPIOA
#define LCDRCC_CLK  RCC_APB2Periph_GPIOA
#define LCD_RST			GPIO_Pin_5
#define LCD_CE			GPIO_Pin_1
#define LCD_DC			GPIO_Pin_2
#define LCD_MOSI		GPIO_Pin_3
#define LCD_CLK			GPIO_Pin_4


//#define LCD_CTRL_PORT	GPIOE
//#define LCDRCC_CLK  RCC_APB2Periph_GPIOE
//#define LCD_RST			GPIO_Pin_10
//#define LCD_CE			GPIO_Pin_9
//#define LCD_DC			GPIO_Pin_12
//#define LCD_MOSI		GPIO_Pin_11
//#define LCD_CLK			GPIO_Pin_13
//---------------------------------------------------------------------------------------------------------------------------------------------


#define LCD_RST_H		GPIO_SetBits(LCD_CTRL_PORT, LCD_RST)
#define LCD_RST_L		GPIO_ResetBits(LCD_CTRL_PORT, LCD_RST)

#define LCD_CE_H		GPIO_SetBits(LCD_CTRL_PORT, LCD_CE)
#define LCD_CE_L		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CE)

#define LCD_DC_DATA		GPIO_SetBits(LCD_CTRL_PORT, LCD_DC)
#define LCD_DC_CMD		GPIO_ResetBits(LCD_CTRL_PORT, LCD_DC)

#define LCD_MOSI_H		GPIO_SetBits(LCD_CTRL_PORT, LCD_MOSI)
#define LCD_MOSI_L		GPIO_ResetBits(LCD_CTRL_PORT, LCD_MOSI)

#define LCD_CLK_H		GPIO_SetBits(LCD_CTRL_PORT, LCD_CLK)
#define LCD_CLK_L		GPIO_ResetBits(LCD_CTRL_PORT, LCD_CLK)

// #define LCD_VCC_H		GPIO_SetBits(LCD_CTRL_PORT, LCD_VCC)

// #define	LCD_BGL_H		GPIO_SetBits(LCD_CTRL_PORT, LCD_BGL)

// #define LCD_GND_L		GPIO_ResetBits(LCD_CTRL_PORT, LCD_GND)


void LCD5110_GPIOInit(void);
void LCD5110_Init(void)	;
void LCD5110_SetContrast(u8 contrast);
void LCD5110_SetXY(u8 X, u8 Y);
void LCD5110Clear(void);
void LCD5110ClearPart(u8 X,u8 Y,u8 L,u8 H);
void LCD5110_Send(u8 data, DCType dc);
void LCD5110WriteChar(u8 X, u8 Y,u8 ascii);
void LCD5110WriteEnStr(u8 X, u8 Y, u8* s);
u32 mypow(u8 m,u8 n);
void LCD5110ShowNum(u8 X,u16 Y,u32 num);
void LCD5110WriteChStr(u8 X, u8 Y,u8 n);

#endif

