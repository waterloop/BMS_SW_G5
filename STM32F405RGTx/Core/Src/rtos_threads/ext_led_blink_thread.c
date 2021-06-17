#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"

#include "main.h"
#include "peripherals.h"
#include "threads.h"

const osThreadAttr_t ext_led_blink_thread_attrs = {
	.name = "ext_led_blink_thread",
	.priority = (osPriority_t)osPriorityIdle
};

void ext_led_blink_thread(void* arg) {
	while (1) {
		HAL_GPIO_TogglePin(EXT_LED_GPIO_Port, EXT_LED_Pin);
		osDelay(1000);
	}
}