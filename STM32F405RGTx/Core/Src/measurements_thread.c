#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"

#include "bms_entry.h"
#include "peripherals.h"
#include "ltc6813.h"
#include "threads.h"

const osThreadAttr_t measurements_thread_attrs = {
	.name = "measurements_thread",
	.priority = (osPriority_t)osPriorityAboveNormal
};

void measurements_thread_fn(void* arg) {
	Ltc6813 slave_device = Ltc6813_init(hspi1, GPIOB, 12);

	Ltc6813_wakeup_sleep(&slave_device);
	uint8_t success;
	HAL_Delay(1000);

	while (1) {
		Ltc6813_wakeup_sleep(&slave_device);

		uart1_print("CFG A");

		success = Ltc6813_read_cfga(&slave_device);

		if (success) {
			uart1_print("PEC SUCCESS");
		} else {
			uart1_print("PEC FAIL");
		}

		Buffer_print(&(slave_device.cfga_bfr));

		uart1_print("CFG B");

		success = Ltc6813_read_cfgb(&slave_device);

		if (success) {
			uart1_print("PEC SUCCESS");
		} else {
			uart1_print("PEC FAIL");
		}

		Buffer_print(&(slave_device.cfgb_bfr));

		HAL_Delay(500);
	}
}

