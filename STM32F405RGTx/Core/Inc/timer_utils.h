#ifndef __TIMER_UTILS
#define __TIMER_UTILS

#include <stdint.h>
#include "main.h"

void start_timers();

void delay_us(uint16_t us);		// uses timer 2 as a 1us counter


#endif
