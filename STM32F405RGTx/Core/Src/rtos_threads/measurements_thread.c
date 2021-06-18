#include <stdio.h>
#include <stdint.h>

#include "cmsis_os.h"
#include "main.h"
#include "peripherals.h"
#include "ltc6813.h"
#include "threads.h"

const osThreadAttr_t measurements_thread_attrs = {
	.name = "measurements_thread",
	.priority = (osPriority_t)osPriorityAboveNormal,
	.stack_size = 1024;
};

void measurements_thread_fn(void* arg) {
	Ltc6813 slave_device = Ltc6813_init(hspi1, GPIOB, 12);

	Ltc6813_wakeup_sleep(&slave_device);
	uint8_t success;
	osDelay(1000);

	while (1) {
		Ltc6813_wakeup_sleep(&slave_device);

		printf("CFG A\r\n");

		success = Ltc6813_read_cfga(&slave_device);

		if (success) {
			printf("PEC SUCCESS\r\n");
		} else {
			printf("PEC FAIL\r\n");
		}

		Buffer_print(&(slave_device.cfga_bfr));

		printf("CFG B\r\n");

		success = Ltc6813_read_cfgb(&slave_device);

		if (success) {
			printf("PEC SUCCESS\r\n");
		} else {
			printf("PEC FAIL\r\n");
		}

		Buffer_print(&(slave_device.cfgb_bfr));

		osDelay(500);
	}
}

