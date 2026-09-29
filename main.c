#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "BIT_MATH.h"
#include "UART.h"
#include "SPI.h"
#include "RFID.h"
#include "twi.h"
#include "OLED.h"

#define B_PIN   PC3  /* Buzzer output pin */
#define LED_PIN PC0  /* Status LED output pin */

static void OLED_PrintHexByte(uint8_t byte) {
	const char hex_chars[] = "0123456789ABCDEF";
	SSD1306_PrintChar(hex_chars[(byte >> 4) & 0x0F]); /* Print high nibble */
	SSD1306_PrintChar(hex_chars[byte & 0x0F]);        /* Print low nibble */
}

static void Display_ReadyScreen(void) {
	SSD1306_Clear();
	SSD1306_SetCursor(0, 16);
	SSD1306_PrintString("RFID SCANNER");
	
	SSD1306_SetCursor(3, 10);
	SSD1306_PrintString("STATUS: READY");
	
	SSD1306_SetCursor(5, 10);
	SSD1306_PrintString("SCAN CARD...");
}

int main(void) {
	RFID_Card card;

	/* 1. Configure output indicators (LED & Buzzer) */
	SET_BIT(DDRC, B_PIN);
	SET_BIT(DDRC, LED_PIN);
	CLR_BIT(PORTC, B_PIN);
	CLR_BIT(PORTC, LED_PIN);

	/* 2. Initialize serial communication (UART) at 9600 baud */
	UART_Init(9600);
	UART_SendString("RFID System Starting...\r\n");

	/* 3. Initialize hardware SPI and RC522 RFID reader */
	SPI_Init();
	RFID_Init();

	/* 4. Initialize TWI/I2C at 400kHz before OLED init */
	TWI_Master_Init(400000);
	SSD1306_Init();

	/* Render initial idle/ready layout */
	Display_ReadyScreen();

	while (1) {
		/* Check if an RFID card is present in the RF field */
		if (RFID_IsCardPresent() == RFID_OK) {
			
			/* Attempt to read UID and card metadata */
			if (RFID_ReadUID(&card) == RFID_OK) {
				
				/* Turn on alert indicators */
				SET_BIT(PORTC, LED_PIN);
				SET_BIT(PORTC, B_PIN);

				/* Update OLED layout for detection state */
				SSD1306_Clear();
				SSD1306_SetCursor(0, 16);
				SSD1306_PrintString("CARD DETECTED");

				/* Page 2: Title label */
				SSD1306_SetCursor(2, 0);
				SSD1306_PrintString("UID:");

				/* Page 4: Put the hex bytes clearly on line 4 */
				SSD1306_SetCursor(4, 0);

				/* Transmit UID over UART and print it directly onto the OLED */
				UART_SendString("Card UID: ");
				for (uint8_t i = 0; i < card.size; i++) {
					/* Send byte to serial monitor via UART */
					UART_SendHex(card.uid[i]);
					UART_SendString(" ");

					/* Render byte on OLED display */
					OLED_PrintHexByte(card.uid[i]);
					SSD1306_PrintChar(' ');
				}
				UART_SendString("\r\n");

				/* Halt card communication to prevent repeated triggering */
				RFID_Halt();

				/* Hold alert feedback for 1 second */
				_delay_ms(1000);
				CLR_BIT(PORTC, LED_PIN);
				CLR_BIT(PORTC, B_PIN);

				/* Return to standby interface */
				Display_ReadyScreen();
			}
		}

		/* Short polling interval */
		_delay_ms(100);
	}
	return 0;
}