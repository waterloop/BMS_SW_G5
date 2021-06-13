#include "cmsis_os.h"

#include "peripherals.h"
#include "timer_utils.h"
#include "bms_entry.h"

#include "threads.h"


void uart1_print(char* char_arr) {
	HAL_UART_Transmit(&huart1, (unsigned char*)(char_arr), strlen(char_arr), 500);
}

void blinky_loop() {
	// test code, blinks an LED on PC3
	GPIOC->MODER &= ~(0b11u << (3*2));
	GPIOC->MODER |= (0b01u << (3*2));

	while (1) {
		GPIOC->ODR ^= (1u << 3);
		HAL_Delay(1000);
	}
}

void delay_us_test() {
	// bit-bangs a 1 MHz square wave on PC3
	GPIOC->MODER &= ~(0b11u << (3*2));
	GPIOC->MODER |= (0b01u << (3*2));

	GPIOC->OSPEEDR &= ~(0b11u << (3*2));
	GPIOC->OSPEEDR |= (0b11u << (3*2));

	while (1) {
		GPIOC->ODR ^= (1u << 3);
		delay_us(1);
	}
}

int bms_entry() {	
	uart1_print("initializing...\r\n");
	
	start_timers();
	osKernelInitialize();

	osThreadNew(measurements_thread_fn, NULL, &measurements_thread_attrs);

	osKernelStart();

	while (1) { asm("NOP"); }	// this line should never execute

	return 0;
}
