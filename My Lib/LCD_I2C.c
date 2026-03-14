#include "LCD_I2C.h"
#include <string.h>

/* Function send 4-bit data */
static void LCD_Send4Bits(LCD_HandleTypeDef *LCDx, uint8_t data, uint8_t mode)
{
    uint8_t high_nibble = mode | (data & 0xF0) | LCDx->backlight;
    uint8_t low_nibble = mode | ((data << 4) & 0xF0) | LCDx->backlight;

    uint8_t data_tx[4];

    // Send high nibble
    data_tx[0] = high_nibble | LCD_EN_SET;  // EN = 1
    data_tx[1] = high_nibble & ~LCD_EN_SET; // EN = 0

    // Send low nibble
    data_tx[2] = low_nibble | LCD_EN_SET;  // EN = 1
    data_tx[3] = low_nibble & ~LCD_EN_SET; // EN = 0

    HAL_I2C_Master_Transmit(LCDx->lcd_hi2c, LCDx->lcd_addr << 1, data_tx, 4, 100);
}

/* Function send command */
static void LCD_SendCommand(LCD_HandleTypeDef *LCDx, uint8_t cmd)
{
    LCD_Send4Bits(LCDx, cmd, RS_COMMAND);
}

/* Function send data */
static void LCD_SendData(LCD_HandleTypeDef *LCDx, uint8_t data)
{
    LCD_Send4Bits(LCDx, data, RS_DATA);
}

void LCD_Init(LCD_HandleTypeDef *LCDx, I2C_HandleTypeDef *hi2c, uint8_t addr)
{
    LCDx->lcd_hi2c = hi2c;
    LCDx->lcd_addr = addr;
    LCDx->backlight = LCD_BACKLIGHT;
    LCDx->displaycontrol = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
    LCDx->displaymode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT;

    HAL_Delay(50); // Wait for LCD to power up

    // Initialization sequence
    LCD_Send4Bits(LCDx, 0x33, RS_COMMAND); // Initialize to 8-bit mode
    HAL_Delay(5);
    LCD_Send4Bits(LCDx, 0x32, RS_COMMAND); // Switch to 4-bit mode
    HAL_Delay(5);

    // Configure LCD
    LCD_SendCommand(LCDx, LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
    LCD_SendCommand(LCDx, LCD_ENTRY_MODE_SET | LCDx->displaymode);

    LCD_Clear_Display(LCDx);
    LCD_Home(LCDx);
}

void LCD_Clear_Display(LCD_HandleTypeDef *LCDx)
{
    LCD_SendCommand(LCDx, LCD_CLEAR_DISPLAY);
    HAL_Delay(2); // Clear display needs delay
}

void LCD_Home(LCD_HandleTypeDef *LCDx)
{
    LCD_SendCommand(LCDx, LCD_RETURN_HOME);
    HAL_Delay(2); // Return home needs delay
}

void LCD_SetCursor(LCD_HandleTypeDef *LCDx, uint8_t col, uint8_t row)
{
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > 3)
        row = 3;
    LCD_SendCommand(LCDx, LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

void LCD_NoDisplay(LCD_HandleTypeDef *LCDx)
{
    LCDx->displaycontrol &= ~LCD_DISPLAY_ON;
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
}

void LCD_Display(LCD_HandleTypeDef *LCDx)
{
    LCDx->displaycontrol |= LCD_DISPLAY_ON;
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
}

void LCD_NoCursor(LCD_HandleTypeDef *LCDx)
{
    LCDx->displaycontrol &= ~LCD_CURSOR_ON;
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
}

void LCD_Cursor(LCD_HandleTypeDef *LCDx)
{
    LCDx->displaycontrol |= LCD_CURSOR_ON;
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
}

void LCD_NoBlink(LCD_HandleTypeDef *LCDx)
{
    LCDx->displaycontrol &= ~LCD_BLINK_ON;
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
}

void LCD_Blink(LCD_HandleTypeDef *LCDx)
{
    LCDx->displaycontrol |= LCD_BLINK_ON;
    LCD_SendCommand(LCDx, LCD_DISPLAY_CONTROL | LCDx->displaycontrol);
}

void LCD_Backlight(LCD_HandleTypeDef *LCDx)
{
    LCDx->backlight = LCD_BACKLIGHT;
    // Send empty command to update backlight
    LCD_Send4Bits(LCDx, 0x00, RS_COMMAND);
}

void LCD_NoBacklight(LCD_HandleTypeDef *LCDx)
{
    LCDx->backlight = LCD_NOBACKLIGHT;
    // Send empty command to update backlight
    LCD_Send4Bits(LCDx, 0x00, RS_COMMAND);
}

void LCD_Print(LCD_HandleTypeDef *LCDx, char *str)
{
    while (*str)
    {
        LCD_PrintChar(LCDx, *str++);
    }
}

void LCD_PrintChar(LCD_HandleTypeDef *LCDx, char ch)
{
    LCD_SendData(LCDx, ch);
}

void LCD_CreateChar(LCD_HandleTypeDef *LCDx, uint8_t location, uint8_t charmap[])
{
    location &= 0x7;
    LCD_SendCommand(LCDx, LCD_SET_CGRAM_ADDR | (location << 3));
    for (int i = 0; i < 8; i++)
    {
        LCD_SendData(LCDx, charmap[i]);
    }
}

// void LCD_ScrollText(LCD_HandleTypeDef *LCDx, uint8_t row, char *message, uint16_t delay_ms)
// {
//     uint8_t len = strlen(message);
//     if (len <= 16)
//     {
//         LCD_SetCursor(LCDx, 0, row);
//         LCD_Print(LCDx, message);
//         return;
//     }

//     for (uint8_t i = 0; i <= len - 16; i++)
//     {
//         LCD_SetCursor(LCDx, 0, row);
//         for (uint8_t j = 0; j < 16; j++)
//         {
//             LCD_PrintChar(LCDx, message[i + j]);
//         }
//         HAL_Delay(delay_ms);
//     }
// }

void LCD_ScrollText(LCD_HandleTypeDef *LCDx, uint8_t row, char *message, uint16_t delay_ms)
{
    uint8_t len = strlen(message);
    uint8_t width = 16; /* LCD width */

    if (len <= width)
    {
        LCD_SetCursor(LCDx, 0, row);
        LCD_Print(LCDx, message);
        return;
    }

    for (uint8_t i = 0; i <= len - width; i++)
    {
        LCD_SetCursor(LCDx, 0, row);
        char buf[17];
        strncpy(buf, &message[i], width);
        buf[width] = '\0';
        LCD_Print(LCDx, buf);
        HAL_Delay(delay_ms);
    }
}