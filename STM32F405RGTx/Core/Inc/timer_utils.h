#pragma once

#include <stdint.h>
#include "main.h"

#define ARR_VAL    65535

void set_led_pwm_dc(uint8_t ch, float dc);
void start_timers();
void delay_us(uint16_t us);     // uses timer 2 as a 1us counter

