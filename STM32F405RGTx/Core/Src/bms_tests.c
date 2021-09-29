#include <stdio.h>
#include "timer_utils.h"
#include "ltc6813.h"

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

void ltc6813_test() {
	Ltc6813 slave_device = Ltc6813_init(hspi1, GPIOA, 4);

	Ltc6813_wakeup_sleep(&slave_device);
	uint8_t success;
	HAL_Delay(1000);

	while (1) {
		Ltc6813_wakeup_sleep(&slave_device);

		printf("CFG A\r\n");

		success = Ltc6813_read_cfga(&slave_device);

		if (success) {
			printf("PEC SUCCESS\r\n");
		} else {
			printf("PEC FAIL\r\n");
		}

		Buffer_print(&(slave_device.cfga_bfr));

//		printf("WRITE REFON 1\r\n");
//
//		uint8_t CFGAR0 = Buffer_index(&(slave_device.cfga_bfr), 0);
//		CFGAR0 |= 0b00000100u;
//
//		Buffer_set_index(&(slave_device.cfga_bfr), 0, CFGAR0);
//
//		Ltc6813_write_cfga(&slave_device);

		HAL_Delay(1000);

		Ltc6813_wakeup_sleep(&slave_device);

		printf("START ADC CONV\r\n");

		success = Ltc6813_read_adc(&slave_device, NORMAL_ADC);

		printf("FINISH ADC CONV\r\n");

		if (success) {
			printf("PEC SUCCESS\r\n");
		} else {
			printf("PEC FAIL\r\n");
		}

		Ltc6813_print_voltages(&slave_device);


		HAL_Delay(1000);
	}
}

