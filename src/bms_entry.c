#include "main.h"
#include "bms_entry.h"
#include "ltc6813.h"
#include "stdint.h"
#include "string.h"

void print(unsigned char* char_arr, uint32_t len) {
	extern UART_HandleTypeDef huart1;
	HAL_UART_Transmit(&huart1, char_arr, len, 500);
}

void bad_delay() {
	for (uint32_t i = 0; i < 14000000; i++) { asm("NOP"); }
}

void blinky_loop() {
	// test code, blinks an LED on pin PB10
	GPIOB->MODER &= ~(0b11u << (10*2));
	GPIOB->MODER |= (0b01u << (10*2));

	while (1) {
		GPIOB->ODR ^= (1 << 10);
		bad_delay();	
	}

}

int bms_entry() {
	blinky_loop();
	// slave_device = Ltc6813_init();
	// GPIOB->MODER &= ~(0b11u << (10*2));
	// GPIOB->MODER |= (0b01u << (10*2));

	// unsigned char str[1] = {0x69};
	// while (1) {
	// 	print(str, 1);
	// 	// GPIOB->ODR ^= (1 << 10);
	// 	// bad_delay();
	// 	// Ltc6813_write_spi(&slave_device, )
	// }

	return 0;
}