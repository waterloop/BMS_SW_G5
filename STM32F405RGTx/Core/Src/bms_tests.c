#include "timer_utils.h"

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


