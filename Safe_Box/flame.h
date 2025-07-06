#ifndef FLAME_H
#define FLAME_H

#include <stdint.h>
void    flame_init(void);
uint16_t flame_read_raw(void);
uint8_t flame_detected(void);

#endif
