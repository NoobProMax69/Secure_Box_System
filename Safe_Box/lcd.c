#include "lcd.h"

// internal delay helper
#define PULSE_DELAY() _delay_us(1)
#define POST_PULSE_DELAY() _delay_us(80)

void lcd_init_pinout(void) {
	
	LCD_D7_DDR |= (1 << LCD_D7_BIT);
	LCD_D6_DDR |= (1 << LCD_D6_BIT);
	LCD_D5_DDR |= (1 << LCD_D5_BIT);
	LCD_D4_DDR |= (1 << LCD_D4_BIT);
	
	LCD_RS_DDR |= (1 << LCD_RS_BIT);
	LCD_E_DDR  |= (1 << LCD_E_BIT);
}


static void lcd_write_nibble(uint8_t nibble) {
	
	// D7
	if (nibble & 0x80) {
		LCD_D7_PORT |= (1 << LCD_D7_BIT);
		} 
	else {
		LCD_D7_PORT &= ~(1 << LCD_D7_BIT);
	}

	// D6
	if (nibble & 0x40) {
		LCD_D6_PORT |= (1 << LCD_D6_BIT);
		} 
	else {
		LCD_D6_PORT &= ~(1 << LCD_D6_BIT);
	}

	// D5
	if (nibble & 0x20) {
		LCD_D5_PORT |= (1 << LCD_D5_BIT);
		} 
	else {
		LCD_D5_PORT &= ~(1 << LCD_D5_BIT);
	}

	// D4
	if (nibble & 0x10) {
		LCD_D4_PORT |= (1 << LCD_D4_BIT);
		} 
	else {
		LCD_D4_PORT &= ~(1 << LCD_D4_BIT);
	}

	// Pulse Enable
	LCD_E_PORT |= (1 << LCD_E_BIT);
	PULSE_DELAY();
	LCD_E_PORT &= ~(1 << LCD_E_BIT);
	POST_PULSE_DELAY();
}

void lcd_init(void) {
	lcd_init_pinout();
	_delay_ms(100);
	// Initialization sequence
	LCD_RS_PORT &= ~(1 << LCD_RS_BIT);
	LCD_E_PORT &= ~(1 << LCD_E_BIT);

	// Reset 3 times
	lcd_write_nibble(LCD_FUNCTION_RESET);
	_delay_ms(10);
	lcd_write_nibble(LCD_FUNCTION_RESET);
	_delay_us(200);
	lcd_write_nibble(LCD_FUNCTION_RESET);
	_delay_us(200);

	// Set to 4-bit mode
	lcd_write_nibble(LCD_FUNCTION_SET_4BIT);
	_delay_us(80);

	// Function set, display off, clear, entry, display on
	lcd_putcmd(LCD_FUNCTION_SET_4BIT);
	lcd_putcmd(LCD_DISPLAY_OFF);
	lcd_putcmd(LCD_CLEAR);
	lcd_putcmd(LCD_ENTRY_MODE);
	lcd_putcmd(LCD_DISPLAY_ON);
}

void lcd_putcmd(uint8_t cmd) {
	// Command mode
	LCD_RS_PORT &= ~(1 << LCD_RS_BIT);
	// Send high nibble, then low nibble
	lcd_write_nibble(cmd & 0xF0);
	lcd_write_nibble((cmd << 4) & 0xF0);
	// If clear or home, wait longer
	if (cmd == LCD_CLEAR || cmd == LCD_HOME) {
		_delay_ms(2);
		} 
	else {
		_delay_us(80);
	}
}

void lcd_putc(uint8_t data) {
	// Data mode
	LCD_RS_PORT |= (1 << LCD_RS_BIT);
	lcd_write_nibble(data & 0xF0);
	lcd_write_nibble((data << 4) & 0xF0);
	_delay_us(80);
}

void lcd_puts(uint8_t *s) {
	while (*s) {
		lcd_putc(*s++);
	}
}

void lcd_clear_row(uint8_t row) {
	lcd_putcmd(LCD_SET_CURSOR | row);
	for (uint8_t i = 0; i < 16; i++) {
		lcd_putc(' ');
	}
	lcd_putcmd(LCD_SET_CURSOR | row);
}
