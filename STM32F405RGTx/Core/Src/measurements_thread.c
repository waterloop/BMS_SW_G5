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
<<<<<<< HEAD
	Ltc6813 slave_device = Ltc6813_init(hspi1, GPIOB, 12);

	Ltc6813_wakeup_sleep(&slave_device);
	uint8_t success;
	HAL_Delay(1000);

=======
>>>>>>> merge ltc6813_driver_dev and master
	while (1) {
		asm("NOP");
	}
}

