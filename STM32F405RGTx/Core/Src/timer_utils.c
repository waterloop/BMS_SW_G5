#include <stdint.h>
#include "main.h"
#include "timer_utils.h"

void start_timers() {
//	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_Base_Start(&htim2);
//	// HAL_TIM_Base_Start(&htim3);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

void delay_us(uint16_t us) {
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (__HAL_TIM_GET_COUNTER(&htim2) < us) { asm("NOP"); }
}


