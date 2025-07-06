#include "uart.h"
#include <avr/io.h>
#include <stdio.h>

void uart_init(void) {
	uint16_t ubrr = 103; // 9600 @ 16 MHz
	UBRR0H = ubrr >> 8;
	UBRR0L = ubrr;
	UCSR0B = (1 << TXEN0);              // TX only
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); 
}

void uart_tx(char c) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

void uart_print(const char* s) {
	while (*s) 
		uart_tx(*s++);
}

void uart_print_hex(uint32_t v) {
	char buf[11];
	sprintf(buf, "0x%08lX", v);
	uart_print(buf);
}
