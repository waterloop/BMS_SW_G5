#include <stdio.h>
#include "cmsis_os.h"

#include "main.h"
#include "can.h"
#include "timer_utils.h"
#include "threads.hpp"
#include "state_machine.hpp"
#include "bms_entry.hpp"

//#include "bms_tests.hpp"

// redirect stdin and stdout to UART1
void __io_putchar(uint8_t ch) {
    HAL_UART_Transmit(&huart1, &ch, 1, 0xffff);
}
uint8_t __io_getchar() {
    uint8_t ch;
    HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
    HAL_UART_Transmit(&huart1, &ch, 1, 0xffff);
    return ch;
}

BMS global_bms_data;
Ltc6813 ltc6813;

osThreadId_t measurements_thread;

void BMS::_lv_test_init() {
    mc_cap_voltage = 46;
    contactor_voltage = 46;
    buck_temp = 25;
    battery.voltage = 46;
    battery.current = 15;
    battery.soc = 100;
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        battery.cells[i].voltage = 3.3;
        battery.cells[i].temp = 25;
    }
}

int bms_entry() {
    printf("starting timers...\r\n");
    start_timers();

    printf("initializing CAN bus...\r\n");
    if (CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
    if (CANBus_subscribe(STATE_CHANGE_REQ) != HAL_OK) { Error_Handler(); }

    hcan1.Instance->MCR = 0x60;

    printf("initializing objects...\r\n");
    ltc6813 = Ltc6813_init(hspi1, GPIOC, 4); //changed from base A to C for CS2

    printf("initializing RTOS kernel...\r\n");
    osKernelInitialize();

    global_bms_data._lv_test_init();

    printf("starting RTOS threads...\r\n");
    measurements_thread = osThreadNew(
        measurements_thread_fn, NULL, &measurements_thread_attrs);

    CoulombCountingThread::initialize();
    StateMachineThread::initialize();

    // RUNNING A BMS test --> Don't start scheduler
    // ltc6813_comm_test();    // Test communication by reading cfg register
    // ltc6813_adc_test();     // Driver test -> Running the ADC

    printf("starting RTOS scheduler...\r\n");
    osKernelStart();

    // should never reach this point, since we have handed execution over to the RTOS
    Error_Handler();

    return 0;
}
