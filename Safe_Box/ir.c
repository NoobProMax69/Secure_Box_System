#include "ir.h"
#include "uart.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

/* --- Pin Definitions --- */
#define IR_PIN_REG  PIND
#define IR_PIN      PD3
#define IR_DDR      DDRD
#define IR_PORT     PORTD

/* --- Timer1 0.5us per tick --- */
static void timer1_init(void) {
	TCCR1A = 0;
	TCCR1B = (1<<CS11);
	TCNT1  = 0;
}

void ir_init(void) {
	IR_DDR  &= ~(1<<IR_PIN);
	IR_PORT |=  (1<<IR_PIN);
	timer1_init();
	uart_print("IR poll init on D3\n");
}

uint32_t ir_read_code(void) {
	static uint32_t last_code = 0;
	uint32_t code = 0;

	uint16_t leader_mark, leader_space, t_space;

	// --- Wait leader mark (LOW) ---
	while (IR_PIN_REG & (1<<IR_PIN));  // wait HIGH to LOW
	TCNT1 = 0;
	while (!(IR_PIN_REG & (1<<IR_PIN))); // wait LOW to HIGH
	leader_mark = TCNT1/2;              

	// --- Measure leader space (HIGH) ---
	TCNT1 = 0;
	while ((IR_PIN_REG & (1<<IR_PIN)));  // wait HIGH to LOW
	leader_space = TCNT1/2;             

	// --- Filter NEC repeat frame ---
	if (leader_space > 2000 && leader_space < 3000) {
		return last_code;
	}

	// --- Read 32 data bits ---
	for (uint8_t i = 0; i < 32; i++) {
		// wait LOW to HIGH (mark end)
		while (!(IR_PIN_REG & (1<<IR_PIN)));
		// measure HIGH until next LOW
		TCNT1 = 0;
		while ((IR_PIN_REG & (1<<IR_PIN)));
		t_space = TCNT1/2;  

		code <<= 1;
		if (t_space > 1000) // >1ms is logical 1
		code |= 1;
	}

	last_code = code;
	return code;
}
