#include "cmsis_os.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "bms_entry.h"
#include "peripherals.h"
#include "ltc6813.h"
#include "threads.h"

const osThreadAttr_t measurements_thread_attrs = {
	.name = "measurements_thread",
	.priority = (osPriority_t)osPriorityAboveNormal
};

void measurements_thread_fn(void* arg) {
	while (1) {
		asm("NOP");
	}
}

