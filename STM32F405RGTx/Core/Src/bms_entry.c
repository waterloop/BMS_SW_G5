#include <stdio.h>
#include "cmsis_os.h"

#include "main.h"
#include "timer_utils.h"
#include "threads.h"
#include "bms_entry.h"

#if BMS_DEBUG
#include "bms_tests.h"
#endif

// redirect stdin and stdout to UART1
void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 0xffff);
}
uint8_t __io_getchar() {
	uint8_t ch;
	HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
	HAL_UART_Transmit(&huart1, &ch, 1, 0xffff);
	return ch;
}

BMS global_bms_data;
Ltc6813 ltc6813;

int bms_entry() {	
	printf("starting timers...\r\n");
	start_timers();	

	printf("initializing objects...\r\n");
	ltc6813 = Ltc6813_init(hspi1, GPIOA, 4);

	printf("initializing RTOS kernel...\r\n");
	osKernelInitialize();

	printf("starting RTOS threads...\r\n");
	ext_led_blink_thread = osThreadNew(ext_led_blink_thread_fn, NULL, &ext_led_blink_thread_attrs);
	measurements_thread = osThreadNew(measurements_thread_fn, NULL, &measurements_thread_attrs);
	fsm_thread = osThreadNew(fsm_thread_fn, NULL, &fsm_thread_attrs);

	printf("starting RTOS scheduler...\r\n");
	osKernelStart();

	// should never reach this point, since we have handed execution over to the RTOS
	Error_Handler();

	return 0;
}
