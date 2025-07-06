#ifndef IR_H
#define IR_H

#include <stdint.h>

// Call once in main()
void    ir_init(void);

// Blocks until a full 32-bit NEC code has been received,
// then returns it.
uint32_t ir_read_code(void);

#endif
