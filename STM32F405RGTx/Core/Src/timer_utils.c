#include <stdint.h>
#include "main.h"
#include "timer_utils.h"

void start_timers() {
	HAL_TIM_Base_Start(&htim2);
}

void delay_us(uint16_t us) {
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (__HAL_TIM_GET_COUNTER(&htim2) < us) { asm("NOP"); }
}


