#ifndef LCD_N_H_
#define LCD_N_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

// LCD control pins (RS and E)
#define LCD_RS_PORT PORTC
#define LCD_RS_DDR  DDRC
#define LCD_RS_BIT  PC0    // A0

#define LCD_E_PORT  PORTC
#define LCD_E_DDR   DDRC
#define LCD_E_BIT   PC1    // A1

// LCD data pins (D4–D7)
#define LCD_D4_PORT PORTD
#define LCD_D4_DDR  DDRD
#define LCD_D4_BIT  PD4

#define LCD_D5_PORT PORTD
#define LCD_D5_DDR  DDRD
#define LCD_D5_BIT  PD5

#define LCD_D6_PORT PORTD
#define LCD_D6_DDR  DDRD
#define LCD_D6_BIT  PD6

#define LCD_D7_PORT PORTD
#define LCD_D7_DDR  DDRD
#define LCD_D7_BIT  PD7

// LCD commands and addresses
#define FIRST_ROW					0x00
#define SECOND_ROW					0x40
#define LCD_CLEAR					0x01
#define LCD_HOME					0x02
#define LCD_ENTRY_MODE				0x06
#define LCD_DISPLAY_OFF				0x08
#define LCD_DISPLAY_ON				0x0C
#define LCD_FUNCTION_RESET			0x30
#define LCD_FUNCTION_SET_4BIT		0x28
#define LCD_SET_CURSOR				0x80

// Function prototypes
void lcd_write(uint8_t byte);
void lcd_putcmd(uint8_t cmd);
void lcd_putc(uint8_t data);
void lcd_puts(uint8_t *str);
void lcd_init_pinout(void);
void lcd_init(void);
void lcd_clear_row(uint8_t row);

#endif /* LCD_N_H_ */
