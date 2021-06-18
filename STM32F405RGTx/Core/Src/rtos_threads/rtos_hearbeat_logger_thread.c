#include <stdio.h>
#include <stdint.h>

#include "cmsis_os.h"
#include "main.h"
#include "peripherals.h"
#include "threads.h"

const osThreadAttr_t rtos_heartbeat_logger_thread_attrs = {
	.name = "rtos_heartbeat_logger_thread",
	.priority = (osPriority_t)osPriorityIdle
};

void rtos_heartbeat_logger_thread_fn(void* arg) {
	while (1) {
		printf("rtos is still running... \r\n");
		osDelay(10000);
	}
}
