#include "flame.h"
#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

#define CAL_SAMPLES 101
#define FLAME_MARGIN_PCT 10

static uint16_t flame_baseline;

// Comparator for qsort
static int cmp_uint16(const void *a, const void *b) {
	uint16_t va = *(const uint16_t*)a;
	uint16_t vb = *(const uint16_t*)b;
	return (va > vb) - (va < vb);
}

void flame_init(void) {
	// ADC setup on A4
	ADMUX  = (1 << REFS0) | (4 & 0x0F);
	DIDR0 |= (1 << ADC4D);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128

	uart_print("Calibrating baseline...");

	// Discard initial samples
	for (uint8_t i = 0; i < 8; i++) {
		ADCSRA |= (1 << ADSC);
		while (ADCSRA & (1 << ADSC));
		_delay_ms(10);
	}

	// Collect calibration samples
	uint16_t readings[CAL_SAMPLES];
	for (uint16_t i = 0; i < CAL_SAMPLES; i++) {
		ADCSRA |= (1 << ADSC);
		while (ADCSRA & (1 << ADSC));
		readings[i] = ADC;
		_delay_ms(10);
	}

	// Sort and pick median as baseline
	qsort(readings, CAL_SAMPLES, sizeof(uint16_t), cmp_uint16);
	flame_baseline = readings[CAL_SAMPLES / 2];

	char buf[64];
	sprintf(buf, "Baseline = %u", flame_baseline);
	uart_print(buf);
}

uint16_t flame_read_raw(void) {
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

uint8_t flame_detected(void) {
	uint16_t v = flame_read_raw();

	char buf[32];
	sprintf(buf, "Raw ADC = %u", v);
	uart_print(buf);

	uint16_t margin = flame_baseline * FLAME_MARGIN_PCT / 100;
	if (v < flame_baseline - margin) {
		return 1;  // flame detected
	}
	return 0;
}
