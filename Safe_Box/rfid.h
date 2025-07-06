#ifndef RFID_H
#define RFID_H

#include <stdint.h>

#define RFID_OK     1
#define RFID_ERROR  0

void rfid_init(void);
uint8_t rfid_check_for_card(uint8_t *uid);

#endif
