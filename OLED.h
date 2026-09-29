#ifndef OLED_H_
#define OLED_H_

#include <avr/io.h>

/* --- SSD1306 I2C Device Address (7-bit address without R/W bit) --- */
#define SSD1306_I2C_ADDR        0x3C

/* --- Display Dimensions --- */
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

/* --- I2C Control Bytes --- */
#define SSD1306_CTRL_CMD        0x00  /* Next byte is a configuration command */
#define SSD1306_CTRL_DATA       0x40  /* Next byte is display data written to GDDRAM */

/* --- Function Prototypes --- */

/**
 * @brief Initialize the SSD1306 OLED display using standard startup sequence.
 */
void SSD1306_Init(void);

/**
 * @brief Send a single command byte to configure internal display registers.
 * @param cmd The command byte to transmit.
 */
void SSD1306_Command(uint8_t cmd);

/**
 * @brief Send a single data byte to be drawn directly into GDDRAM.
 * @param data 8-bit vertical pixel slice (Bit 0 = top pixel, Bit 7 = bottom pixel).
 */
void SSD1306_Data(uint8_t data);

void SSD1306_SetCursor(uint8_t page, uint8_t column);

/**
 * @brief Clear the entire 128x64 display memory by filling GDDRAM with zeros.
 */
void SSD1306_Clear(void);

/**
 * @brief Print a single ASCII character onto the display using 5x7 font.
 * @param ch The character to render.
 */
void SSD1306_PrintChar(char ch);

/**
 * @brief Print a null-terminated string onto the display.
 * @param str Pointer to the character array.
 */
void SSD1306_PrintString(const char *str);

#endif /* SSD1306_H_ */