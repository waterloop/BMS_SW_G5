#include <stdio.h>
#include <stdint.h>

#include "cmsis_os.h"
#include "main.h"
#include "threads.h"

const osThreadAttr_t fsm_thread_attrs = {
	.name = "state_machine_thread",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 1024
};

void fsm_thread_fn(void* arg) {
	while (1) {
		printf("buck temp = %.4f\r\n", global_bms_data.buck_temp);
		printf("mc cap voltage = %.4f\r\n", global_bms_data.mc_cap_voltage);
		printf("contactor voltage = %.4f\r\n", global_bms_data.contactor_voltage);
		printf("battery voltage = %.4f\r\n", global_bms_data.battery.voltage);
		printf("battery current = %.4f\r\n", global_bms_data.battery.current);
		printf("soc = %.4f\r\n\r\n", global_bms_data.battery.soc);

		osDelay(500);
	}
}
