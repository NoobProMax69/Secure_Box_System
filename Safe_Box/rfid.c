#include "rfid.h"
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include <stdio.h>

/* --- Pin Definitions --- */
#define RST_PIN   PB1  
#define NSS_PIN   PB2  
#define MOSI_PIN  PB3  
#define MISO_PIN  PB4  
#define SCK_PIN   PB5  

/* --- RC522 Register Definitions --- */
#define CommandReg        0x01
#define CommIEnReg        0x02
#define CommIrqReg        0x04
#define ErrorReg          0x06
#define Status1Reg        0x07
#define FIFODataReg       0x09
#define FIFOLevelReg      0x0A
#define ControlReg        0x0C
#define BitFramingReg     0x0D
#define ModeReg           0x11
#define TxASKReg          0x15
#define TModeReg          0x2A
#define TPrescalerReg     0x2B
#define TReloadRegH       0x2C
#define TReloadRegL       0x2D
#define VersionReg        0x37
#define TxControlReg      0x14


#define PICC_REQIDL       0x26
#define PCD_TRANSCEIVE    0x0C


/* --- SPI --- */
static void spi_init(void) {
	DDRB |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << NSS_PIN) | (1 << RST_PIN);
	DDRB &= ~(1 << MISO_PIN);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);  // Fosc / 64
	DDRB |= (1 << NSS_PIN);  // NSS as output

}

static uint8_t spi_transfer(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

/* --- RFID Chip Select --- */
static void rfid_select(void) {
	PORTB &= ~(1 << NSS_PIN);
	_delay_us(10); 
}

static void rfid_unselect(void) {
	_delay_us(10); // data clocked out
	PORTB |= (1 << NSS_PIN);
}

/* --- Register Access --- */
static void rfid_write_reg(uint8_t reg, uint8_t val) {
	rfid_select();
	spi_transfer((reg << 1) & 0x7E);
	spi_transfer(val);
	rfid_unselect();
}

static uint8_t rfid_read_reg(uint8_t reg) {
	rfid_select();
	spi_transfer((reg << 1) | 0x80);
	uint8_t val = spi_transfer(0);
	rfid_unselect();
	return val;
}

static void rfid_set_bitmask(uint8_t reg, uint8_t mask) {
	rfid_write_reg(reg, rfid_read_reg(reg) | mask);
}

static void rfid_clear_bitmask(uint8_t reg, uint8_t mask) {
	rfid_write_reg(reg, rfid_read_reg(reg) & (~mask));
}


/* --- Card Communication --- */

static uint8_t rfid_to_card(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backBits) {

	uint8_t status = RFID_ERROR;
	uint8_t irqEn = 0x77;
	uint8_t waitIRq = 0x30;
	uint8_t lastBits, n;
	uint16_t i;

	rfid_write_reg(CommIEnReg, irqEn | 0x80);
	rfid_clear_bitmask(CommIrqReg, 0x80);
	rfid_set_bitmask(FIFOLevelReg, 0x80);

	for (i = 0; i < sendLen; i++) {
		rfid_write_reg(FIFODataReg, sendData[i]);
	}

	rfid_write_reg(CommandReg, command);
	if (command == PCD_TRANSCEIVE)
		rfid_set_bitmask(BitFramingReg, 0x80);

	i = 2000;
	do {
		n = rfid_read_reg(CommIrqReg);
		i--;
	} while (i && !(n & 0x01) && !(n & waitIRq));

	// Debug print if timeout happens
	if (i == 0) {
		uart_print("Timeout waiting for response\n");

		// If SPI still alive, check VersionReg again
		uint8_t version = rfid_read_reg(VersionReg);
		char buf[32];
		sprintf(buf, "VersionReg during timeout: 0x%02X\n", version);
		uart_print(buf);
	}
	uint8_t err = rfid_read_reg(ErrorReg);
	char errbuf[32];
	sprintf(errbuf, "ErrorReg: 0x%02X\n", err);
	uart_print(errbuf);

	rfid_clear_bitmask(BitFramingReg, 0x80);

	if (i && !(rfid_read_reg(ErrorReg) & 0x1B)) {
		status = RFID_OK;
		if (command == PCD_TRANSCEIVE) {
			uint8_t count = rfid_read_reg(FIFOLevelReg);
			lastBits = rfid_read_reg(ControlReg) & 0x07;
			if (lastBits) {
				*backBits = (count - 1) * 8 + lastBits;
				} 
			else {
				*backBits = count * 8;
			}
			for (i = 0; i < count; i++) {
				backData[i] = rfid_read_reg(FIFODataReg);
			}
		}
	}

	return status;
}


void rfid_init(void) {
	spi_init();

	// Reset the RC522
	PORTB |= (1 << RST_PIN);
	_delay_ms(50);
	PORTB &= ~(1 << RST_PIN);
	_delay_ms(50);
	PORTB |= (1 << RST_PIN);

	// Soft reset
	rfid_write_reg(CommandReg, 0x0F);
	_delay_ms(50);

	// RC522 init settings (from datasheet)
	rfid_write_reg(TModeReg, 0x8D);
	rfid_write_reg(TPrescalerReg, 0x3E);
	rfid_write_reg(TReloadRegL, 0x1E);
	rfid_write_reg(TReloadRegH, 0x00);
	rfid_write_reg(TxASKReg, 0x40);
	rfid_write_reg(ModeReg, 0x3D);
	
	// Power on the antenna manually
	uint8_t val = rfid_read_reg(TxControlReg);
	uart_print("TxControlReg before: ");
	char buf[10];
	sprintf(buf, "0x%02X\n", val);
	uart_print(buf);

	// Set antenna on
	rfid_set_bitmask(TxControlReg, 0x03);

	val = rfid_read_reg(TxControlReg);
	uart_print("TxControlReg after: ");
	sprintf(buf, "0x%02X\n", val);
	uart_print(buf);

}



uint8_t rfid_check_for_card(uint8_t *uid) {
	for (uint8_t i = 0; i < 5; i++) uid[i] = 0;

	uint8_t tagType[2];
	uint8_t backBits;

	// Wake up any tag
	rfid_write_reg(BitFramingReg, 0x07);  // Send 7 bits
	uint8_t req_cmd = 0x26;
	if (rfid_to_card(PCD_TRANSCEIVE, &req_cmd, 1, tagType, &backBits) != RFID_OK) {
		return RFID_ERROR;
	}

	// Expect 16 bits
	if (backBits != 0x10) 
		return RFID_ERROR;

	// Anti-collision: Get UID
	rfid_write_reg(BitFramingReg, 0x00);  
	uint8_t antiColl[] = {0x93, 0x20};
	if (rfid_to_card(PCD_TRANSCEIVE, antiColl, 2, uid, &backBits) != RFID_OK)
		return RFID_ERROR;

	// Must receive 40 bits
	if (backBits != 40) 
		return RFID_ERROR;

	uint8_t checksum = 0;
	for (uint8_t i = 0; i < 4; i++) 
		checksum ^= uid[i];

	if (checksum == uid[4]) {
		return RFID_OK;
		} else {
		return RFID_ERROR;
	}

}
