#include "main.h"
#include "bms_entry.h"
#include "stdint.h"

void shitty_delay() {
	for (uint32_t i = 0; i < 9000; i++) { asm("NOP"); }
}

int bms_entry() {
	GPIOA->MODER &= ~(0b11u << (10*2));
	GPIOA->MODER |= (0b01u << (10*2));

	while (1) {
		GPIOA->ODR |= (1 << 10);
		shitty_delay();
	}

	return 0;
}