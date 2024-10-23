/******************************************************************************************************************
@File:  	CLCD 8BIT (Character LCD 8Bit Mode)
@Author:  Khue Nguyen
@Website: khuenguyencreator.com
@Youtube: https://www.youtube.com/channel/UCt8cFnPOaHrQXWmVkk-lfvg

Huong dan su dung:
- Su dung thu vien HAL
- Khoi tao bien LCD: CLCD_Name LCD1;
- Khoi tao LCD do:
Che do 8 bit
CLCD_8BIT_Init(&LCD1, 16, 2, CS_GPIO_Port, CS_Pin, EN_GPIO_Port, EN_Pin,
									D0_GPIO_Port, D0_Pin, D1_GPIO_Port, D1_Pin,
									D2_GPIO_Port, D2_Pin, D3_GPIO_Port, D3_Pin,
									D4_GPIO_Port, D4_Pin, D5_GPIO_Port, D5_Pin,
									D6_GPIO_Port, D6_Pin, D7_GPIO_Port, D7_Pin);
Che do 4 bit
CLCD_4BIT_Init(&LCD1, 16, 2, CS_GPIO_Port, CS_Pin, EN_GPIO_Port, EN_Pin,
									D4_GPIO_Port, D4_Pin, D5_GPIO_Port, D5_Pin,
									D6_GPIO_Port, D6_Pin, D7_GPIO_Port, D7_Pin);
- Su dung cac ham truyen dia chi cua LCD do:
CLCD__SetCursor(&LCD1, 0, 0);
CLCD_WriteString(&LCD1,"Hello anh em");
******************************************************************************************************************/
#include "lcd.h"


//************************ Low Level Function *****************************************//
static void CLCD_Delay(uint16_t Time)
{
	HAL_Delay(Time);

}
void CLCD_I2C_Init(LCD_I2C_HandleTypeDef *p_LCD, I2C_HandleTypeDef *p_hi2c, uint8_t p_col, uint8_t p_row, uint8_t p_Slave_Address)
{
	p_LCD->SLAVE_ADDRESS = p_Slave_Address;
	p_LCD->LCD_Backlight_Value = lcd_backlight_on;
	p_LCD->LCD_Columns = p_col;
	p_LCD->LCD_Rows = p_row;
	p_LCD->hi2c = p_hi2c;
	p_LCD->LCD_Display_Option = lcd_display_on | lcd_cursor_off | lcd_blinkOff;


	//4 bit initialization
	HAL_Delay(40); 	//waiting for > 40ms
	lcd_send_cmd(p_LCD, 0x30);
	HAL_Delay(5);  // waiting for >4.1ms
	lcd_send_cmd(p_LCD, 0x30);
	HAL_Delay(1);  // waiting for >100us
	lcd_send_cmd(p_LCD, 0x30);
	HAL_Delay(10);
	lcd_send_cmd(p_LCD, 0x20);	//4 bit mode
	HAL_Delay(10);

	//display initialization
	lcd_send_cmd(p_LCD, 0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd(p_LCD, 0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off		HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd(p_LCD, 0x01);  // clear display
	HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd(p_LCD, 0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd(p_LCD, lcd_displaycontrol | p_LCD -> LCD_Display_Option); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	HAL_Delay(1);
}

void lcd_send_cmd (LCD_I2C_HandleTypeDef *p_lcd, char cmd)
{
	char data_h, data_l;
	uint8_t data_t[4];
	data_h = (cmd & 0xf0);
	data_l = ((cmd<<4) & 0xf0);

	data_t[0] = data_h | 0x0C;	//en=1, rs=0
	data_t[1] = data_h | 0x08;	//en=0, rs=0
	data_t[2] = data_l | 0x0C;	//en=1, rs=0
	data_t[3] = data_l | 0x08;	//en=0, rs=1

	HAL_I2C_Master_Transmit(p_lcd->hi2c, p_lcd->SLAVE_ADDRESS, data_t, 4, 100);
}

void lcd_send_data (LCD_I2C_HandleTypeDef *p_LCD, char data)
{
	char data_h, data_l;
	uint8_t data_t[4];
	data_h = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);

	data_t[0] = data_h | 0x0D; //en=1, rs=0
	data_t[1] = data_h | 0x09; //en=0, rs=0
	data_t[2] = data_l | 0x0D;
	data_t[3] = data_l | 0x09;

	HAL_I2C_Master_Transmit(p_LCD -> hi2c, p_LCD->SLAVE_ADDRESS, data_t, 4, 100);
}


void lcd_set_cursor(LCD_I2C_HandleTypeDef *p_LCD, uint8_t p_col, uint8_t p_row)
{
	uint8_t t_row_Offets[] = {0x00, 0x40, 0x14, 0x54};
	if(p_row > p_LCD->LCD_Rows) p_row = p_LCD->LCD_Rows - 1;
	lcd_send_cmd(p_LCD, 0x80 | (p_col + t_row_Offets[p_row]));
}

void lcd_send_string (LCD_I2C_HandleTypeDef *p_LCD, char *str)
{
	while (*str) lcd_send_data(p_LCD, *str++);
}

void lcd_clear (LCD_I2C_HandleTypeDef *p_LCD)
{
	lcd_send_cmd(p_LCD, lcd_cleardisplay);
	HAL_Delay(5);
	lcd_set_cursor(p_LCD, 0, 0);
}
static void CLCD_Write8(CLCD_Name* LCD, uint8_t Data, uint8_t Mode)
{
	if(Mode == CLCD_COMMAND)
	{
		HAL_GPIO_WritePin(LCD->RS_PORT, LCD->RS_PIN, GPIO_PIN_RESET);// RS = 0, write cmd
	}
	else if(Mode == CLCD_DATA)
	{
		HAL_GPIO_WritePin(LCD->RS_PORT, LCD->RS_PIN, GPIO_PIN_SET);// RS = 1, write data
	}
	HAL_GPIO_WritePin(LCD->D0_PORT, LCD->D0_PIN, Data&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D1_PORT, LCD->D1_PIN, Data>>1&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D2_PORT, LCD->D2_PIN, Data>>2&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D3_PORT, LCD->D3_PIN, Data>>3&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D4_PORT, LCD->D4_PIN, Data>>4&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D5_PORT, LCD->D5_PIN, Data>>5&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D6_PORT, LCD->D6_PIN, Data>>6&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D7_PORT, LCD->D7_PIN, Data>>7&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);

	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_RESET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_SET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_RESET);
	CLCD_Delay(1);
}
static void CLCD_Write4(CLCD_Name* LCD, uint8_t Data, uint8_t Mode)
{
	uint8_t Data_H = Data >>4;
	uint8_t Data_L = Data;
	if(Mode == CLCD_COMMAND)
	{
		HAL_GPIO_WritePin(LCD->RS_PORT, LCD->RS_PIN, GPIO_PIN_RESET);// RS = 0, write cmd
	}
	else if(Mode == CLCD_DATA)
	{
		HAL_GPIO_WritePin(LCD->RS_PORT, LCD->RS_PIN, GPIO_PIN_SET);// RS = 1, write data
	}
	HAL_GPIO_WritePin(LCD->D4_PORT, LCD->D4_PIN, Data_H&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D5_PORT, LCD->D5_PIN, Data_H>>1&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D6_PORT, LCD->D6_PIN, Data_H>>2&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D7_PORT, LCD->D7_PIN, Data_H>>3&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);

	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_RESET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_SET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_RESET);
	CLCD_Delay(1);

	HAL_GPIO_WritePin(LCD->D4_PORT, LCD->D4_PIN, Data_L&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D5_PORT, LCD->D5_PIN, Data_L>>1&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D6_PORT, LCD->D6_PIN, Data_L>>2&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD->D7_PORT, LCD->D7_PIN, Data_L>>3&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);

	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_RESET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_SET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD->EN_PORT, LCD->EN_PIN, GPIO_PIN_RESET);
	CLCD_Delay(1);
}
//************************ High Level Function *****************************************//
void CLCD_8BIT_Init(CLCD_Name* LCD, uint8_t Colum, uint8_t Row,
									GPIO_TypeDef* RS_PORT, uint16_t RS_PIN, GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
									GPIO_TypeDef* D0_PORT, uint16_t D0_PIN, GPIO_TypeDef* D1_PORT, uint16_t D1_PIN,
									GPIO_TypeDef* D2_PORT, uint16_t D2_PIN, GPIO_TypeDef* D3_PORT, uint16_t D3_PIN,
									GPIO_TypeDef* D4_PORT, uint16_t D4_PIN, GPIO_TypeDef* D5_PORT, uint16_t D5_PIN,
									GPIO_TypeDef* D6_PORT, uint16_t D6_PIN, GPIO_TypeDef* D7_PORT, uint16_t D7_PIN)
{
	LCD->MODE = LCD_8BITMODE;
	LCD->COLUMS = Colum;
	LCD->ROWS = Row;
	LCD->RS_PORT = RS_PORT;
	LCD->RS_PIN = RS_PIN;
	LCD->EN_PORT = EN_PORT;
	LCD->EN_PIN = EN_PIN;
	LCD->D0_PORT = D0_PORT;
	LCD->D0_PIN = D0_PIN;
	LCD->D1_PORT = D1_PORT;
	LCD->D1_PIN = D1_PIN;
	LCD->D2_PORT = D2_PORT;
	LCD->D2_PIN = D2_PIN;
	LCD->D3_PORT = D3_PORT;
	LCD->D3_PIN = D3_PIN;
	LCD->D4_PORT = D4_PORT;
	LCD->D4_PIN = D4_PIN;
	LCD->D5_PORT = D5_PORT;
	LCD->D5_PIN = D5_PIN;
	LCD->D6_PORT = D6_PORT;
	LCD->D6_PIN = D6_PIN;
	LCD->D7_PORT = D7_PORT;
	LCD->D7_PIN = D7_PIN;
	CLCD_Delay(50);
	LCD->FUNCTIONSET = LCD_FUNCTIONSET|LCD_8BITMODE|LCD_2LINE|LCD_5x8DOTS;
	LCD->ENTRYMODE = LCD_ENTRYMODESET|LCD_ENTRYLEFT|LCD_ENTRYSHIFTDECREMENT;
	LCD->DISPLAYCTRL = LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_CURSOROFF|LCD_BLINKOFF;
	LCD->CURSORSHIFT = LCD_CURSORSHIFT|LCD_CURSORMOVE|LCD_MOVERIGHT;

	CLCD_Write8(LCD, LCD->ENTRYMODE,CLCD_COMMAND);
	CLCD_Write8(LCD, LCD->DISPLAYCTRL,CLCD_COMMAND);
	CLCD_Write8(LCD, LCD->CURSORSHIFT,CLCD_COMMAND);
	CLCD_Write8(LCD, LCD->FUNCTIONSET,CLCD_COMMAND);

	CLCD_Write8(LCD, LCD_CLEARDISPLAY,CLCD_COMMAND);
	CLCD_Write8(LCD, LCD_RETURNHOME,CLCD_COMMAND);
}

void CLCD_4BIT_Init(CLCD_Name* LCD, uint8_t Colum, uint8_t Row,
									GPIO_TypeDef* RS_PORT, uint16_t RS_PIN, GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
									GPIO_TypeDef* D4_PORT, uint16_t D4_PIN, GPIO_TypeDef* D5_PORT, uint16_t D5_PIN,
									GPIO_TypeDef* D6_PORT, uint16_t D6_PIN, GPIO_TypeDef* D7_PORT, uint16_t D7_PIN)
{
	LCD->MODE = LCD_4BITMODE;
	LCD->COLUMS = Colum;
	LCD->ROWS = Row;
	LCD->RS_PORT = RS_PORT;
	LCD->RS_PIN = RS_PIN;
	LCD->EN_PORT = EN_PORT;
	LCD->EN_PIN = EN_PIN;
	LCD->D4_PORT = D4_PORT;
	LCD->D4_PIN = D4_PIN;
	LCD->D5_PORT = D5_PORT;
	LCD->D5_PIN = D5_PIN;
	LCD->D6_PORT = D6_PORT;
	LCD->D6_PIN = D6_PIN;
	LCD->D7_PORT = D7_PORT;
	LCD->D7_PIN = D7_PIN;
	LCD->FUNCTIONSET = LCD_FUNCTIONSET|LCD_4BITMODE|LCD_2LINE|LCD_5x8DOTS;
	LCD->ENTRYMODE = LCD_ENTRYMODESET|LCD_ENTRYLEFT|LCD_ENTRYSHIFTDECREMENT;
	LCD->DISPLAYCTRL = LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_CURSOROFF|LCD_BLINKOFF;
	LCD->CURSORSHIFT = LCD_CURSORSHIFT|LCD_CURSORMOVE|LCD_MOVERIGHT;

	CLCD_Delay(50);
	CLCD_Write4(LCD, 0x33, CLCD_COMMAND);
	CLCD_Delay(5);
	CLCD_Write4(LCD, 0x33, CLCD_COMMAND);
	CLCD_Delay(5);
	CLCD_Write4(LCD, 0x32, CLCD_COMMAND);
	CLCD_Delay(5);

	CLCD_Write4(LCD, LCD->ENTRYMODE,CLCD_COMMAND);
	CLCD_Write4(LCD, LCD->DISPLAYCTRL,CLCD_COMMAND);
	CLCD_Write4(LCD, LCD->CURSORSHIFT,CLCD_COMMAND);
	CLCD_Write4(LCD, LCD->FUNCTIONSET,CLCD_COMMAND);

	CLCD_Write4(LCD, LCD_CLEARDISPLAY,CLCD_COMMAND);
	CLCD_Write4(LCD, LCD_RETURNHOME,CLCD_COMMAND);
}
void CLCD_SetCursor(LCD_I2C_HandleTypeDef *p_LCD, uint8_t p_col, uint8_t p_row)
{
	uint8_t t_row_Offets[] = {0x00, 0x40, 0x14, 0x54};
	if(p_row > p_LCD->LCD_Rows) p_row = p_LCD->LCD_Rows - 1;
	lcd_send_cmd(p_LCD, 0x80 | (p_col + t_row_Offets[p_row]));
}
void CLCD_WriteChar(LCD_I2C_HandleTypeDef *p_LCD, char data)
{
	char data_h, data_l;
	uint8_t data_t[4];
	data_h = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);

	data_t[0] = data_h | 0x0D; //en=1, rs=0
	data_t[1] = data_h | 0x09; //en=0, rs=0
	data_t[2] = data_l | 0x0D;
	data_t[3] = data_l | 0x09;

	HAL_I2C_Master_Transmit(p_LCD -> hi2c, p_LCD->SLAVE_ADDRESS, data_t, 4, 100);
}
void CLCD_WriteString(LCD_I2C_HandleTypeDef *p_LCD, char *str)
{
	while (*str) lcd_send_data(p_LCD, *str++);
}
void CLCD_Clear(LCD_I2C_HandleTypeDef *p_LCD)
{
	lcd_send_cmd(p_LCD, lcd_cleardisplay);
	HAL_Delay(5);
	lcd_set_cursor(p_LCD, 0, 0);
}
void CLCD_ReturnHome(CLCD_Name* LCD)
{
	if(LCD->MODE == LCD_8BITMODE)
	{
		CLCD_Write8(LCD, LCD_RETURNHOME, CLCD_COMMAND);
	}
	else if(LCD->MODE == LCD_4BITMODE)
	{
		CLCD_Write4(LCD, LCD_RETURNHOME, CLCD_COMMAND);
	}
	CLCD_Delay(5);
}
void CLCD_CursorOn(CLCD_Name* LCD)
{
	LCD->DISPLAYCTRL |= LCD_CURSORON;
	if(LCD->MODE == LCD_8BITMODE)
	{
		CLCD_Write8(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
	else if(LCD->MODE == LCD_4BITMODE)
	{
		CLCD_Write4(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
}
void CLCD_CursorOff(CLCD_Name* LCD)
{
	LCD->DISPLAYCTRL &= ~LCD_CURSORON;
	if(LCD->MODE == LCD_8BITMODE)
	{
		CLCD_Write8(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
	else if(LCD->MODE == LCD_4BITMODE)
	{
		CLCD_Write4(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
}
void CLCD_BlinkOn(CLCD_Name* LCD)
{
	LCD->DISPLAYCTRL |= LCD_BLINKON;
	if(LCD->MODE == LCD_8BITMODE)
	{
		CLCD_Write8(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
	else if(LCD->MODE == LCD_4BITMODE)
	{
		CLCD_Write4(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
}
void CLCD_BlinkOff(CLCD_Name* LCD)
{
	LCD->DISPLAYCTRL &= ~LCD_BLINKON;
	if(LCD->MODE == LCD_8BITMODE)
	{
		CLCD_Write8(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}
	else if(LCD->MODE == LCD_4BITMODE)
	{
		CLCD_Write4(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
	}

}
