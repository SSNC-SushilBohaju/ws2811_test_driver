#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H

#include "common-defines.h"
#include<time.h>

#define CPU_FREQ      (1400000000) // Typical Raspberry Pi CPU frequency in Hz (1.5 GHz)
#define SYSTICK_FREQ  (1000)       // 1 ms tick

void system_setup(void);
uint64_t system_get_ticks(void);

#endif // INC_SYSTEM_H
