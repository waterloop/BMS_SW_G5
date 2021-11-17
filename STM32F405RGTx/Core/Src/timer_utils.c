#include <stdint.h>
#include "main.h"
#include "timer_utils.h"

void set_led_pwm_dc(uint8_t ch, float dc) {
    uint32_t ccr_val = (uint32_t)( ((100 - dc)*ARR_VAL)/100 );
    switch (ch) {
        case 1:
            htim1.Instance->CCR1 = ccr_val;
        case 2:
            htim1.Instance->CCR2 = ccr_val;
        case 3:
            htim1.Instance->CCR3 = ccr_val;
        case 4:
            htim1.Instance->CCR4 = ccr_val;
    }
}

void start_timers() {
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Base_Start(&htim2);
    // HAL_TIM_Base_Start(&htim3);

    set_led_pwm_dc(1, 0);
    set_led_pwm_dc(2, 0);
    set_led_pwm_dc(3, 0);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us) { asm("NOP"); }
}


