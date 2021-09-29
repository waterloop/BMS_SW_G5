#include <stdio.h>
#include <stdint.h>

#include "cmsis_os.h"
#include "main.h"
#include "threads.h"
#include "bms_tests.h"

/*
TODO:
	- this thread is just used for testing, eventually it should be removed
*/

const osThreadAttr_t debug_log_thread_attrs = {
	.name = "debug_log_thread",
	.priority = (osPriority_t)osPriorityLow,
	.stack_size = 1024
};

void debug_log_thread_fn(void* arg) {
	{
		// intentionally make "str" go out of scope
		char str[10];
		printf("press enter to turn on contactor: ");
		scanf(str);
		printf("\r\n\r\n");
	}

	printf("turning on contactor in");
	printf("\r\n3...");
	osDelay(1000);
	printf("\r\n2...");
	osDelay(1000);
	printf("\r\n1...");
	osDelay(1000);

	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1);
	printf("\r\ncontactor has been turned on...\r\n\r\n");
	printf("reading %.2fV at contactor...\r\n", global_bms_data.contactor_voltage);

	osDelay(500);
	while (1) {
		printf("buck temp = %.2f\r\n", global_bms_data.buck_temp);
		printf("mc cap voltage = %.2f\r\n", global_bms_data.mc_cap_voltage);
		printf("contactor voltage = %.2f\r\n", global_bms_data.contactor_voltage);
		printf("battery voltage = %.2f\r\n", global_bms_data.battery.voltage);
		printf("battery current = %.2f\r\n", global_bms_data.battery.current);
		printf("soc = %.2f\r\n\r\n", global_bms_data.battery.soc);

		osDelay(500);
	}
}

