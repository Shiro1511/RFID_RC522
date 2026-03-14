/**
 ******************************************************************************
 * @file           : LCD_I2C.h
 * @brief          : Header file for HD44780-compatible LCD via I2C interface.
 *                   Provides functions to initialize, control, and print text
 *                   on LCD modules using a PCF8574 I/O expander.
 ******************************************************************************
 * @attention
 *
 * This module allows control of standard 16x2, 20x4 LCD displays connected
 * through an I2C adapter (e.g. PCF8574). It supports basic text display,
 * cursor control, backlight management, and custom character creation.
 *
 * Example usage:
 * @code
 *   LCD_HandleTypeDef hlcd;
 *   LCD_Init(&hlcd, &hi2c1, LCD_ADDR);
 *   LCD_Backlight(&hlcd);
 *   LCD_SetCursor(&hlcd, 0, 0);
 *   LCD_Print(&hlcd, "Hello, world!");
 * @endcode
 *
 ******************************************************************************
 */

#ifndef _LCD_I2C_H_
#define _LCD_I2C_H_

#include "stm32f1xx_hal.h"

/* -------------------------------------------------------------------------- */
/*                               LCD Constants                                */
/* -------------------------------------------------------------------------- */

/* LCD's I2C Default address */
#define LCD_ADDR 0x27

/* Command LCD */
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_CGRAM_ADDR 0x40
#define LCD_SET_DDRAM_ADDR 0x80

/* Flags for display entry mode */
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

/* Flags for display on/off control */
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

/* Flags for display/cursor shift */
#define LCD_DISPLAY_MOVE 0x08
#define LCD_CURSOR_MOVE 0x00
#define LCD_MOVE_RIGHT 0x04
#define LCD_MOVE_LEFT 0x00

/* Flags for function set */
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2LINE 0x08
#define LCD_5x8_DOTS 0x00

/* Backlight control */
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

/* Enable bit */
#define LCD_EN_SET 0x04
#define LCD_EN_CLEAR 0x00

#define DISABLE 0x00
#define RS_COMMAND 0x00
#define RS_DATA 0x01

/* -------------------------------------------------------------------------- */
/*                             LCD Handle Struct                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief  LCD handle structure definition
 */
typedef struct
{
    I2C_HandleTypeDef *lcd_hi2c; /*!< Pointer to I2C handler */
    uint8_t lcd_addr;            /*!< I2C address of LCD */
    uint8_t backlight;           /*!< Backlight control flag */
    uint8_t displaycontrol;      /*!< Display control flags */
    uint8_t displaymode;         /*!< Display mode flags */
} LCD_HandleTypeDef;

/* -------------------------------------------------------------------------- */
/*                              Function Prototypes                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Initialize the LCD module via I2C.
 * @param  LCDx: Pointer to LCD handle structure.
 * @param  hi2c: Pointer to I2C handle (e.g. &hi2c1).
 * @param  addr: I2C address of LCD (e.g. LCD_ADDR).
 * @retval None
 */
void LCD_Init(LCD_HandleTypeDef *LCDx, I2C_HandleTypeDef *hi2c, uint8_t addr);

/**
 * @brief  Clear the LCD display and return cursor to home position.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_Clear_Display(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Set cursor to home position (0,0).
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_Home(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Set cursor to specific position on LCD.
 * @param  LCDx: Pointer to LCD handle structure.
 * @param  col: Column position (0-based).
 * @param  row: Row position (0-based).
 * @retval None
 */
void LCD_SetCursor(LCD_HandleTypeDef *LCDx, uint8_t col, uint8_t row);

/**
 * @brief  Turn off the LCD display (contents are saved).
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_NoDisplay(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn on the LCD display.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_Display(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn off the underline cursor.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_NoCursor(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn on the underline cursor.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_Cursor(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn off blinking cursor.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_NoBlink(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn on blinking cursor.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_Blink(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn on LCD backlight.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_Backlight(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Turn off LCD backlight.
 * @param  LCDx: Pointer to LCD handle structure.
 * @retval None
 */
void LCD_NoBacklight(LCD_HandleTypeDef *LCDx);

/**
 * @brief  Print a string on the LCD starting at current cursor position.
 * @param  LCDx: Pointer to LCD handle structure.
 * @param  str: Pointer to null-terminated string.
 * @retval None
 */
void LCD_Print(LCD_HandleTypeDef *LCDx, char *str);

/**
 * @brief  Print a single character on the LCD.
 * @param  LCDx: Pointer to LCD handle structure.
 * @param  ch: Character to display.
 * @retval None
 */
void LCD_PrintChar(LCD_HandleTypeDef *LCDx, char ch);

/**
 * @brief  Create a custom character in CGRAM.
 * @param  LCDx: Pointer to LCD handle structure.
 * @param  location: Memory slot (0–7) for custom character.
 * @param  charmap: 8-byte pattern defining the character.
 * @retval None
 */
void LCD_CreateChar(LCD_HandleTypeDef *LCDx, uint8_t location, uint8_t charmap[]);

/**
 * @brief  Scroll text horizontally across a given LCD row.
 * @param  LCDx: Pointer to LCD handle structure.
 * @param  row: LCD row index (0 or 1 for 16x2).
 * @param  message: Pointer to text string to scroll.
 * @param  delay_ms: Delay time between scroll steps in milliseconds.
 * @retval None
 */
void LCD_ScrollText(LCD_HandleTypeDef *LCDx, uint8_t row, char *message, uint16_t delay_ms);

#endif /* _LCD_I2C_H_ */