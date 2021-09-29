#include <stdio.h>
#include <stdint.h>

#include "cmsis_os.h"
#include "main.h"
#include "threads.h"

#define ARR_VAL	65535

const osThreadAttr_t ext_led_blink_thread_attrs = {
	.name = "ext_led_blink_thread",
	.priority = (osPriority_t)osPriorityIdle
};

void _set_ch_duty_cycle(uint8_t ch, float dc) {
	uint32_t ccr_val = (uint32_t)( ((100 - dc)*ARR_VAL)/100 );
	switch (ch) {
		case 1:
			htim1.Instance->CCR1 = ccr_val;
		case 2:
			htim1.Instance->CCR2 = ccr_val;
		case 3:
			htim1.Instance->CCR3 = ccr_val;
		case 4:
			htim1.Instance->CCR4 = ccr_val;
	}
}

void ext_led_blink_thread_fn(void* arg) {
	while (1) {
		_set_ch_duty_cycle(1, 100);
		_set_ch_duty_cycle(2, 0);
		_set_ch_duty_cycle(3, 0);
	
		osDelay(500);

		_set_ch_duty_cycle(1, 0);
		_set_ch_duty_cycle(2, 100);
		_set_ch_duty_cycle(3, 0);

		osDelay(500);

		_set_ch_duty_cycle(1, 50);
		_set_ch_duty_cycle(2, 0);
		_set_ch_duty_cycle(3, 50);

		osDelay(500);
	}
}
