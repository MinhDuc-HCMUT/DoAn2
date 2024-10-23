/*
 * keypad4x4.h
 *
 *  Created on: May 4, 2024
 *      Author: Minh Duc
 */

#ifndef INC_KEYPAD4X4_H_
#define INC_KEYPAD4X4_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

/*------------------------Define your GPIO pins here - START----------------------------*/

/**
  Keypad	STM32	       	 Description         I/O
     L1			PB4				Row 1           Output
     L2			PB5				Row 2           Output
     L3			PB6				Row 3           Output
     L4			PB7				Row 4           Output

     R1			PB0				Column 1        Input
     R2			PB1	 			Column 2        Input
     R3			PB2				Column 3        Input
     R4			PB3				Column 4        Input
**/
/* Rows  ~ OUTPUT */
/* Row 1 default */
#define KEYPAD_ROW_1_PORT			  GPIOB
#define KEYPAD_ROW_1_PIN			  GPIO_PIN_0
/* Row 2 default */
#define KEYPAD_ROW_2_PORT			  GPIOB
#define KEYPAD_ROW_2_PIN			  GPIO_PIN_1
/* Row 3 default */
#define KEYPAD_ROW_3_PORT			  GPIOB
#define KEYPAD_ROW_3_PIN			  GPIO_PIN_3
/* Row 4 default */
#define KEYPAD_ROW_4_PORT			  GPIOB
#define KEYPAD_ROW_4_PIN			  GPIO_PIN_4

/* Columns ~ INPUT */
/* Column 1 default */
#define KEYPAD_COLUMN_1_PORT		GPIOB
#define KEYPAD_COLUMN_1_PIN			GPIO_PIN_5
/* Column 2 default */
#define KEYPAD_COLUMN_2_PORT		GPIOB
#define KEYPAD_COLUMN_2_PIN			GPIO_PIN_6
/* Column 3 default */
#define KEYPAD_COLUMN_3_PORT		GPIOB
#define KEYPAD_COLUMN_3_PIN			GPIO_PIN_7
/* Column 4 default */
#define KEYPAD_COLUMN_4_PORT		GPIOB
#define KEYPAD_COLUMN_4_PIN			GPIO_PIN_8

/*------------------------Define your GPIO pins here - END----------------------------*/
/* Keypad NOT pressed */
#define KEYPAD_NOT_PRESSED			'\0'


/**
 * @brief  Reads keypad data
 * @param  None
 * @retval Button status. This parameter will be a value of KEYPAD_Button_t enumeration
 */
char KEYPAD_Read(void);

#endif /* INC_KEYPAD4X4_H_ */
