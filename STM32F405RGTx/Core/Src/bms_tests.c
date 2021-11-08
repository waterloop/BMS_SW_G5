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
    while(1) {
        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        Ltc6813_read_adc(&ltc6813, NORMAL_ADC);
        Ltc6813_print_voltages(&ltc6813);
    }
}

