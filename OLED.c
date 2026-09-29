#include "OLED.h"
#include "twi.h"

/* 
 * 5x7 Font Lookup Table:
 * Each character is represented by 5 vertical byte slices.
 * Bit 0 corresponds to the topmost pixel, and Bit 6 corresponds to the bottom.
 */
static const uint8_t Font5x7[][5] = {
    [' ' - ' '] = {0x00, 0x00, 0x00, 0x00, 0x00}, /* Space */
    ['0' - ' '] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1' - ' '] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2' - ' '] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3' - ' '] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4' - ' '] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5' - ' '] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6' - ' '] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7' - ' '] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8' - ' '] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9' - ' '] = {0x06, 0x49, 0x49, 0x29, 0x1E},
    ['A' - ' '] = {0x7C, 0x12, 0x11, 0x12, 0x7C},
    ['B' - ' '] = {0x7F, 0x49, 0x49, 0x49, 0x36},
    ['C' - ' '] = {0x3E, 0x41, 0x41, 0x41, 0x22},
    ['D' - ' '] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
    ['E' - ' '] = {0x7F, 0x49, 0x49, 0x49, 0x41},
    ['F' - ' '] = {0x7F, 0x09, 0x09, 0x09, 0x01},
    ['H' - ' '] = {0x7F, 0x08, 0x08, 0x08, 0x7F},
    ['I' - ' '] = {0x00, 0x41, 0x7F, 0x41, 0x00},
    ['L' - ' '] = {0x7F, 0x40, 0x40, 0x40, 0x40},
    ['O' - ' '] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
    ['R' - ' '] = {0x7F, 0x09, 0x19, 0x29, 0x46},
    ['S' - ' '] = {0x46, 0x49, 0x49, 0x49, 0x31},
    ['T' - ' '] = {0x01, 0x01, 0x7F, 0x01, 0x01},
    [':']       = {0x00, 0x36, 0x36, 0x00, 0x00}
};

void SSD1306_Command(uint8_t cmd) {
    TWI_Start();
    TWI_Write((SSD1306_I2C_ADDR << 1) | 0); /* Send SLA+W (Write mode) */
    TWI_Write(SSD1306_CTRL_CMD);             /* Set Control Byte to Command Mode */
    TWI_Write(cmd);                          /* Write command payload */
    TWI_Stop();
}

void SSD1306_Data(uint8_t data) {
    TWI_Start();
    TWI_Write((SSD1306_I2C_ADDR << 1) | 0); /* Send SLA+W (Write mode) */
    TWI_Write(SSD1306_CTRL_DATA);            /* Set Control Byte to Data Mode */
    TWI_Write(data);                         /* Write pixel data directly to GDDRAM */
    TWI_Stop();
}

void SSD1306_Init(void) {
    _delay_ms(100); /* Allow supply voltage (VCC) to stabilize after power-up */

    /* 1. Turn display off during initial register configuration */
    SSD1306_Command(0xAE);

    /* 2. Set display clock divide ratio and internal oscillator frequency */
    SSD1306_Command(0xD5);
    SSD1306_Command(0x80); /* Default suggested ratio */

    /* 3. Set multiplex ratio for 128x64 display (64 lines total) */
    SSD1306_Command(0xA8);
    SSD1306_Command(0x3F); /* 64MUX (0x3F = 63 + 1 lines) */

    /* 4. Set display offset (no vertical shift) */
    SSD1306_Command(0xD3);
    SSD1306_Command(0x00);

    /* 5. Set display start line to line 0 */
    SSD1306_Command(0x40);

    /* 6. Enable internal DC-DC charge pump (Required: converts 3.3V/5V to ~8V) */
    SSD1306_Command(0x8D);
    SSD1306_Command(0x14); /* 0x14 enables internal charge pump regulator */

    /* 7. Configure memory addressing mode to Horizontal Mode */
    SSD1306_Command(0x20);
    SSD1306_Command(0x00); /* 0x00 = Horizontal Addressing Mode */

    /* 8. Configure panel mapping: Re-map columns (0 to 127) */
    SSD1306_Command(0xA1);

    /* 9. Configure COM output scan direction: Scan from COM[N-1] down to COM0 */
    SSD1306_Command(0xC8);

    /* 10. Set COM pins hardware configuration for alternative layout */
    SSD1306_Command(0xDA);
    SSD1306_Command(0x12);

    /* 11. Set contrast control value (Brightness) */
    SSD1306_Command(0x81);
    SSD1306_Command(0xCF); /* High brightness level */

    /* 12. Set pre-charge period for internal OLED cells */
    SSD1306_Command(0xD9);
    SSD1306_Command(0xF1);

    /* 13. Set VCOMH deselect voltage level */
    SSD1306_Command(0xDB);
    SSD1306_Command(0x40);

    /* 14. Resume display to follow GDDRAM contents */
    SSD1306_Command(0xA4);

    /* 15. Set normal display polarity (Bit 1 = pixel illuminated, Bit 0 = off) */
    SSD1306_Command(0xA6);

    /* 16. Turn display panel ON */
    SSD1306_Command(0xAF);

    /* Clear any random garbage data from memory */
    SSD1306_Clear();
}

void SSD1306_SetCursor(uint8_t page, uint8_t column) {
    if (page > 7) page = 7;
    if (column > 127) column = 127;

    /* Define page range: start at 'page' and end at page 7 */
    SSD1306_Command(0x22);
    SSD1306_Command(page);
    SSD1306_Command(7);

    /* Define column range: start at 'column' and end at column 127 */
    SSD1306_Command(0x21);
    SSD1306_Command(column);
    SSD1306_Command(127);
}

void SSD1306_Clear(void) {
    /* Reset cursor back to top-left corner (Page 0, Column 0) */
    SSD1306_SetCursor(0, 0);

    /* Open a continuous data stream to burst clear all 1024 bytes */
    TWI_Start();
    TWI_Write((SSD1306_I2C_ADDR << 1) | 0);
    TWI_Write(SSD1306_CTRL_DATA);

    for (uint16_t i = 0; i < 1024; i++) {
        TWI_Write(0x00); /* Write zero to turn off all 8 pixels in this vertical slice */
    }
    TWI_Stop();
}

void SSD1306_PrintChar(char ch) {
    uint8_t index = (uint8_t)(ch - ' '); /* Calculate offset relative to ASCII space */

    /* Output the 5 vertical slices defining this character */
    for (uint8_t i = 0; i < 5; i++) {
        SSD1306_Data(Font5x7[index][i]);
    }
    /* Add a 1-pixel blank vertical column for character spacing */
    SSD1306_Data(0x00);
}

void SSD1306_PrintString(const char *str) {
    /* Iterate through characters until reaching the null-terminator */
    while (*str) {
        SSD1306_PrintChar(*str++);
    }
}