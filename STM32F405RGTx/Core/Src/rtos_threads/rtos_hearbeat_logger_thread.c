#include <stdio.h>

#include "cmsis_os.h"
#include "threads.h"

const osThreadAttr_t rtos_heartbeat_logger_thread_attrs = {
	.name = "rtos_heartbeat_logger_thread",
	.priority = (osPriority_t)osPriorityIdle
};
void rtos_heartbeat_logger_thread(void* arg) {
	while (1) {
		printf("rtos is still running...");
		osDelay(10000);
	}
}
