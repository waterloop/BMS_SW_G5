#include "bms_entry.h"
#include "peripherals.h"
#include "ltc6813.h"
#include "timer_utils.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

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
	uart1_print("start...\n");

	start_timers();
	// blinky_loop();
	// delay_us_test();

	Ltc6813 slave_device = Ltc6813_init(hspi2, GPIOB, 12);

	Buffer pkt = Buffer_init();

	// WRCFGA
	// Buffer_append(&pkt, 0b000u);
	// Buffer_append(&pkt, 0b00000001u);
	// for (uint8_t i = 0; i < 40; i++) {
	// 	Buffer_append(&pkt, 0u);
	// }
	// Buffer_set_index(&pkt, 2, 0b11111000u);
	// Buffer_add_pec(&pkt);

	// RDCFGA, just trying to get data back by reading the default configuration register values
	// idk why I didn't think of this before I'm actually trolling
	Buffer_append(&pkt, 0b000u);
	Buffer_append(&pkt, 0b00000010u);
	Buffer_add_pec(&pkt);

	// RDCVA command 
	// Buffer_append(&pkt, 0b000u);
	// Buffer_append(&pkt, 0b00000100u);
	// Buffer_add_pec(&pkt);

	Buffer response_pkt = Buffer_init();
	response_pkt.len = 8*6;

	char str[500];
	for (uint8_t i = 0; i < pkt.len; i++) {
		sprintf(str, "pkt byte %d: %d\n", i, Buffer_index(&pkt, i));
		uart1_print(str);
	}
	uart1_print("\n");

	Ltc6813_wakeup_sleep(&slave_device);

	while (1) {
		// if (Buffer_index(&pkt, 2) == 0b11111000u) {
		// 	Buffer_set_index(&pkt, 2, 0b11110000u);
		// }
		// else {
		// 	Buffer_set_index(&pkt, 2, 0b11111000u);
		// }

		Ltc6813_wakeup_idle(&slave_device);

		Ltc6813_write_spi(&slave_device, &pkt);

		Ltc6813_read_spi(&slave_device, &response_pkt);

		for (uint8_t i = 0; i < response_pkt.len; i++) {
			sprintf(str, "byte %d: %d\n", i, Buffer_index(&response_pkt, i));
			uart1_print(str);
		}
		uart1_print("\n");

		HAL_Delay(500);
	}

	return 0;
}
