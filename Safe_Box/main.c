#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "lcd.h"
#include "rfid.h"
#include "ir.h"
#include "flame.h"

// Button pins (active low)
#define RFID_BTN_PIN   PD2  // D2
#define IR_BTN_PIN     PC5  // A5
#define FIRE_BTN_PIN   PC2  // A2

// LED pins
#define LED_OK_PIN     PB0  // D8
#define LED_ERR_PIN    PC3  // A3

// Clear LEDs
static void clear_leds(void) {
	PORTB &= ~(1 << LED_OK_PIN);
	PORTC &= ~(1 << LED_ERR_PIN);
}

// Display two lines: l1 on row 1, l2 on row 2
static void lcd_display(const char *l1, const char *l2) {
	// Clear display
	lcd_putcmd(LCD_CLEAR);
	_delay_ms(5);
	// Row 1
	lcd_putcmd(LCD_SET_CURSOR | FIRST_ROW);
	_delay_us(80);
	lcd_puts((uint8_t*)l1);
	// Row 2
	lcd_putcmd(LCD_SET_CURSOR | SECOND_ROW);
	_delay_us(80);
	lcd_puts((uint8_t*)l2);
}

int main(void) {
	// I/O setup
	// Buttons
	DDRD  &= ~(1 << RFID_BTN_PIN);
	PORTD |=  (1 << RFID_BTN_PIN);
	DDRC  &= ~((1 << IR_BTN_PIN) | (1 << FIRE_BTN_PIN));
	PORTC |=  (1 << IR_BTN_PIN) | (1 << FIRE_BTN_PIN);
	// LEDs
	DDRB |= (1 << LED_OK_PIN);
	DDRC |= (1 << LED_ERR_PIN);
	clear_leds();

	lcd_init();
	lcd_display("Secure Box"," ");
	_delay_ms(2000);
	rfid_init();
	ir_init();
	flame_init();

	// Edge detection
	uint8_t prev_rfid = 1, prev_ir = 1, prev_fire = 1;

	// RFID expected UID
	uint8_t uid[5];
	const uint8_t expected_uid[5] = {0x47,0xA1,0x31,0x66,0xB1};

	while (1) {
		// Read buttons (0 when pressed)
		uint8_t cur_rfid = !(PIND & (1 << RFID_BTN_PIN));
		uint8_t cur_ir   = !(PINC & (1 << IR_BTN_PIN));
		uint8_t cur_fire = !(PINC & (1 << FIRE_BTN_PIN));

		// RFID mode
		if (cur_rfid && !prev_rfid) {
			clear_leds();
			lcd_display("RFID Mode","Present tag");
			// wait for tag
			while (rfid_check_for_card(uid) == RFID_ERROR);
			// show result
			clear_leds();
			if (memcmp(uid, expected_uid, 5) == 0) {
				lcd_display("RFID Mode","Access granted!");
				PORTB |= (1 << LED_OK_PIN);
				} 

			else {
				lcd_display("RFID Mode","Access denied!");
				PORTC |= (1 << LED_ERR_PIN);
			}
		}

		// IR mode
		if (cur_ir && !prev_ir) {
			clear_leds();
			lcd_display("IR Mode","Enter code");
			_delay_ms(200);
			// capture 4 digits
			char entry[5] = {0};
			uint8_t remote_code = 0;
			while (remote_code < 4) {
				uint32_t code = ir_read_code();
				char d;
				switch (code) {
					case 0x00FF6897: d='1'; break;
					case 0x00FF9867: d='2'; break;
					case 0x00FFB04F: d='3'; break;
					case 0x00FF30CF: d='4'; break;
					case 0x00FF18E7: d='5'; break;
					case 0x00FF7A85: d='6'; break;
					case 0x00FF10EF: d='7'; break;
					case 0x00FF38C7: d='8'; break;
					case 0x00FF5AA5: d='9'; break;
					case 0x00FF4AB5: d='0'; break;
					default: d='?';
				}
				if (d >= '0' && d <= '9') {
					entry[remote_code++] = d;
					_delay_ms(300);
				}
			}
			entry[4] = '\0';
			// show result
			clear_leds();
			if (strcmp(entry,"1993") == 0) {
				lcd_display("IR Mode","Access granted!");
				PORTB |= (1 << LED_OK_PIN);
				} 

			else {
				lcd_display("IR Mode","Access denied!");
				PORTC |= (1 << LED_ERR_PIN);
			}
		}

		// Fire mode
		if (cur_fire && !prev_fire) {
			clear_leds();
			if (flame_detected()) {
				lcd_display("Fire Detector","Fire!");
				PORTC |= (1 << LED_ERR_PIN);
				} 

			else {
				lcd_display("Fire Detector","No fire");
				PORTB |= (1 << LED_OK_PIN);
			}
		}

		// update previous
		prev_rfid = cur_rfid;
		prev_ir   = cur_ir;
		prev_fire = cur_fire;
		_delay_ms(50);
	}
	return 0;
}
