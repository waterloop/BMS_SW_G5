#include <stdio.h>
#include "cmsis_os.h"

#include "main.h"
#include "can.h"
#include "timer_utils.h"
#include "threads.h"
#include "state_machine.h"
#include "bms_entry.h"

//#include "bms_tests.h"

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
osThreadId_t coulomb_counting_thread;
osThreadId_t state_machine_thread;

void _lv_test_init_global_bms_obj() {
    global_bms_data.mc_cap_voltage = 46;
    global_bms_data.contactor_voltage = 46;
    global_bms_data.buck_temp = 25;
    global_bms_data.battery.voltage = 46;
    global_bms_data.battery.current = 15;
    global_bms_data.battery.soc = 100;
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        global_bms_data.battery.cells[i].voltage = 3.3;
        global_bms_data.battery.cells[i].temp = 25;
    }
}

void __report_CAN() {
    CANFrame tx_frame = CANFrame_init(BMS_FAULT_REPORT);
    CANFrame_set_field(&tx_frame, BMS_SEVERITY_CODE, SEVERE);
    CANFrame_set_field(&tx_frame, BMS_ERROR_CODE, LOW_LAYER_EXCEPTION);
}

void __cell_disable() {
    uint32_t cell_mask = 0b0;
    Ltc6813_discharge_ctrl(&ltc6813, cell_mask);
}

void __hard_fault_state_trans() {
    CurrentState = SevereDangerFault;
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

    _lv_test_init_global_bms_obj();

    printf("starting RTOS threads...\r\n");
    measurements_thread = osThreadNew(
        measurements_thread_fn, NULL, &measurements_thread_attrs);

    coulomb_counting_thread = osThreadNew(
        coulomb_counting_thread_fn, NULL, &coulomb_counting_thread_attrs);

    state_machine_thread = osThreadNew(
        StartStateMachine, NULL, &state_machine_thread_attrs);

    // RUNNING A BMS test --> Don't start scheduler
    // ltc6813_comm_test();    // Test communication by reading cfg register
    // ltc6813_adc_test();     // Driver test -> Running the ADC

    printf("starting RTOS scheduler...\r\n");
    osKernelStart();

    // should never reach this point, since we have handed execution over to the RTOS
    Error_Handler();

    return 0;
}
