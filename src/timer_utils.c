#include "peripherals.h"
#include "timer_utils.h"
#include "stdint.h"

void start_timers() {
	HAL_TIM_Base_Start(&htim3);
}

void delay_us(uint16_t us) {
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	while (__HAL_TIM_GET_COUNTER(&htim3) < us) { asm("NOP"); }
}


