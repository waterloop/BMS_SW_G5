<<<<<<< HEAD:STM32F405RGTx/Core/Src/bms_tests.c
#include <stdio.h>
#include "timer_utils.h"
#include "ltc6813.h"

void blinky_loop() {
    // test code, blinks an LED on PB3
    GPIOB->MODER &= ~(0b11u << (3*2));
    GPIOB->MODER |= (0b01u << (3*2));

    while (1) {
        GPIOB->ODR ^= (1u << 3);
        HAL_Delay(1000);
    }
}

void delay_us_test() {
    // bit-bangs a 1 MHz square wave on PB3
    GPIOB->MODER &= ~(0b11u << (3*2));
    GPIOB->MODER |= (0b01u << (3*2));

    GPIOB->OSPEEDR &= ~(0b11u << (3*2));
    GPIOB->OSPEEDR |= (0b11u << (3*2));

    while (1) {
        GPIOB->ODR ^= (1u << 3);
        delay_us(1);
    }
}

void ltc6813_comm_test() {
    while(1) {
        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        Ltc6813_read_adc(&ltc6813, NORMAL_ADC);

        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        uint8_t success = Ltc6813_read_reg(&ltc6813, RDCVA);
        success = Ltc6813_read_reg(&ltc6813, RDCVB);
        success = Ltc6813_read_reg(&ltc6813, RDCVC);
        success = Ltc6813_read_reg(&ltc6813, RDCVD);
        success = Ltc6813_read_reg(&ltc6813, RDCVE);
        success = Ltc6813_read_reg(&ltc6813, RDCVF);
        _Ltc6813_decode_adc(&ltc6813);
        if (success == 1) {
            printf("PEC SUCCESS");
            Buffer_print(&(ltc6813.cfga_bfr));
        } else {
            printf("PEC FAIL");
        }
    }
}

void ltc6813_adc_test() {
    while(1) {
        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        Ltc6813_read_adc(&ltc6813, NORMAL_ADC);
        Ltc6813_print_voltages(&ltc6813);
    }
}
=======
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

void ltc6813_comm_test() {
    while(1) {
        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        Ltc6813_read_adc(&ltc6813, NORMAL_ADC);

        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        uint8_t success = Ltc6813_read_reg(&ltc6813, RDCVA);
        success = Ltc6813_read_reg(&ltc6813, RDCVB);
        success = Ltc6813_read_reg(&ltc6813, RDCVC);
        success = Ltc6813_read_reg(&ltc6813, RDCVD);
        success = Ltc6813_read_reg(&ltc6813, RDCVE);
        success = Ltc6813_read_reg(&ltc6813, RDCVF);
        _Ltc6813_decode_adc(&ltc6813);
        if (success == 1) {
            printf("PEC SUCCESS");
            Buffer_print(&(ltc6813.cfga_bfr));
        } else {
            printf("PEC FAIL");
        }
    }
}

void ltc6813_adc_test() {
    while(1) {
        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        Ltc6813_read_adc(&ltc6813, NORMAL_ADC);
        Ltc6813_print_voltages(&ltc6813);
    }
}
>>>>>>> origin/c++_port:STM32F405RGTx/Core/Src/bms_tests.cpp
