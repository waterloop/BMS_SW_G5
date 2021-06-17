#include <stdio.h>

#include "cmsis_os.h"
#include "peripherals.h"
#include "timer_utils.h"

#include "threads.h"

#if BMS_DEBUG
#include "bms_tests.h"
#endif

// override __io_putchar from the STL to enable UART printing
void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 500);
}

int bms_entry() {	
	printf("starting timers...\r\n");
	start_timers();

	printf("initializing RTOS kernel...\r\n");
	osKernelInitialize();

	printf("starting RTOS threads...\r\n");
	osThreadNew(ext_led_blink_thread_fn, NULL, &ext_led_blink_thread_attrs);
	// osThreadNew(rtos_heartbeat_logger_thread_fn, NULL, &rtos_heartbeat_logger_thread_attrs);
	osThreadNew(measurements_thread_fn, NULL, &measurements_thread_attrs);

	printf("starting RTOS scheduler...\r\n");
	osKernelStart();

	// should never reach this point, since we have handed execution over to the RTOS
	Error_Handler();

	return 0;
}
