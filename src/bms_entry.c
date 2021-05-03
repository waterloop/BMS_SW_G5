#include "bms_entry.h"
#include "peripherals.h"
#include "ltc6813.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

void uart1_print(char* char_arr) {
	HAL_UART_Transmit(&huart1, (unsigned char*)(char_arr), strlen(char_arr), 500);
}

void blinky_loop() {
	// test code, blinks an LED on pin PB10
	GPIOB->MODER &= ~(0b11u << (10*2));
	GPIOB->MODER |= (0b01u << (10*2));

	while (1) {
		GPIOB->ODR ^= (1 << 10);
		HAL_Delay(1000);
	}
}

int bms_entry() {	
	Ltc6813 slave_device = Ltc6813_init(hspi2, GPIOC, 3);

	Buffer pkt = Buffer_init();
	Buffer_append(&pkt, 0b11001100);
	Buffer_append(&pkt, 0b11001100);

	Buffer response_pkt = Buffer_init();
	response_pkt.len = 8;

	char str[500];

	Ltc6813_wakeup_sleep(&slave_device);

	while (1) {
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
