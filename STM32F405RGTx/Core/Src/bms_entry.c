#include <stdio.h>

#include "cmsis_os.h"
#include "peripherals.h"
#include "timer_utils.h"
#include "bms_entry.h"

#include "threads.h"

void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 500);	// override __io_putchar from the STL to enable UART printing
}


int bms_entry() {	
	printf("initializing...\r\n");
	
	start_timers();
	osKernelInitialize();

	osThreadNew(measurements_thread_fn, NULL, &measurements_thread_attrs);

	osKernelStart();

	while (1) { asm("NOP"); }	// this line should never execute

	return 0;
}
