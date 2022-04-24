#include <stdio.h>
#include "cmsis_os.h"

#include "main.h"
#include "can.h"
#include "timer_utils.h"
#include "threads.hpp"
#include "state_machine.hpp"
#include "bms_entry.hpp"

//#include "bms_tests.hpp"

BMS global_bms_data;
Ltc6813 ltc6813;

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

void _report_CAN() {
    CANFrame tx_frame = CANFrame_init(BMS_FAULT_REPORT);
    CANFrame_set_field(&tx_frame, BMS_SEVERITY_CODE, SEVERE);
    CANFrame_set_field(&tx_frame, BMS_ERROR_CODE, LOW_LAYER_EXCEPTION);
}

void _cell_disable() {
    uint32_t cell_mask = 0b0;
    Ltc6813_discharge_ctrl(&ltc6813, cell_mask);
}

void _hard_fault_state_trans() {
    StateMachineThread::setState(SevereDangerFault);
}

int bms_entry() {
    printf("\r\n");
    printf("starting timers...\r\n");
    start_timers();

    // uncomment for debugging, turns on loopback and turns off autoretransmission
    // hcan1.Instance->MCR = 0x60;
    // hcan1.Instance->MCR |= (1 << 4);
    // hcan1.Instance->BTR |= (1 << 30);

    // uncomment for debugging, initializes GPIO for timing GPIO pin
    // GPIOA->MODER &= ~(0b11 << (15*2));
    // GPIOA->MODER |= (0b01 << (15*2));

    printf("initializing objects...\r\n");
    ltc6813 = Ltc6813_init(hspi1, GPIOC, 4); //changed from base A to C for CS2

    printf("initializing RTOS kernel...\r\n");
    osKernelInitialize();

    global_bms_data._lv_test_init();

    printf("starting RTOS threads...\r\n");
    
    MeasurementsThread::initialize();
    CoulombCountingThread::initialize();
    StateMachineThread::initialize();
    Ltc6813Thread::initialize();
    BistThread::initialize();
    LEDThread::initialize();

    // RUNNING A BMS test --> Don't start scheduler
    // ltc6813_comm_test();    // Test communication by reading cfg register
    // ltc6813_adc_test();     // Driver test -> Running the ADC

    printf("starting RTOS scheduler...\r\n");
    osKernelStart();

    // should never reach this point, since we have handed execution over to the RTOS
    Error_Handler();

    return 0;
}
