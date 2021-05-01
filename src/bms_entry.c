#include "main.h"
#include "bms_entry.h"
#include "stdint.h"

void blinky_loop() {
	GPIOB->MODER &= ~(0b11u << (10*2));
	GPIOB->MODER |= (0b01u << (10*2));

	while (1) {
		GPIOB->ODR ^= (1 << 10);
		for (uint32_t i = 0; i < 14000000; i++) { asm("NOP"); }
	}

}

int bms_entry() {
	blinky_loop();

	return 0;
}