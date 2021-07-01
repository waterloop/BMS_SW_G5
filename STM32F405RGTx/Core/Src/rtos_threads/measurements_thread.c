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
	.stack_size = 2048
};

void measurements_thread_fn(void* arg) {
	Ltc6813 slave_device = Ltc6813_init(hspi1, GPIOA, 4);

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

//		printf("WRITE REFON 1\r\n");
//
//		uint8_t CFGAR0 = Buffer_index(&(slave_device.cfga_bfr), 0);
//		CFGAR0 |= 0b00000100u;
//
//		Buffer_set_index(&(slave_device.cfga_bfr), 0, CFGAR0);
//
//		Ltc6813_write_cfga(&slave_device);

		osDelay(1000);

		Ltc6813_wakeup_sleep(&slave_device);

		printf("START ADC CONV\r\n");

		success = Ltc6813_read_adc(&slave_device, NORMAL_ADC);

		printf("FINISH ADC CONV\r\n");

		if (success) {
			printf("PEC SUCCESS\r\n");
		} else {
			printf("PEC FAIL\r\n");
		}

		Ltc6813_print_voltages(&slave_device);


		osDelay(1000);
	}
}

