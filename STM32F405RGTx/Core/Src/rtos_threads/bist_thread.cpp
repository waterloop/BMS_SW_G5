#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "threads.hpp"
#include "bms_entry.hpp"
#include "bist_thread.hpp"

RTOSThread BistThread::thread_;

void BistThread::initialize() {
    BistThread::thread_ = RTOSThread(
            "bist_thread",
            1024,
            osPriorityLow,
            BistThread::runBist
    );
}

void BistThread::runBist(void* args) {
    uint8_t buff[20];
    uint32_t len = 20;
    while (1) {
        BistThread::_sinput((uint8_t*)"> ", buff, &len);

        if (strcmp((const char*)buff, (const char*)"p_measurements") == 0) { BistThread::_p_measurements(); }

        else if (strcmp((const char*)buff, (const char*)"") == 0) { /* do nothing... */ }
        else { printf("invalid command...\r\n"); }

        len = 20;
    }
}

void BistThread::_print(uint8_t* str) {
    uint32_t len = 0;
    while (str[len] != '\0') { len += 1; }

    HAL_UART_Transmit(&huart1, str, len, 1);
}

void BistThread::_sinput(uint8_t* prompt, uint8_t* buff, uint32_t* len) {
    BistThread::_print(prompt);
    uint32_t curr_len = 0;

    while (curr_len < (*len)) {
        uint8_t tmp;

        // try to receive 1 character with a timeout value of 1ms
        HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, &tmp, 1, 1);

        if (status == HAL_TIMEOUT) {
            // do nothing...
        }
        else if (status == HAL_OK) {
            HAL_UART_Transmit(&huart1, &tmp, 1, 1);
            if (tmp == '\n') {
                break;
            }
            else if (tmp == '\r') {
                BistThread::_print((uint8_t*)"\n");
                break;
            }
            else {
                buff[curr_len] = tmp;
                curr_len += 1;
            }
        }
        else if (status == HAL_ERROR) {
            Error_Handler();
        }

        // humans cannot type faster than 50ms per character...
        osDelay(50);
    }
    buff[curr_len++] = '\0';
    *len = curr_len;
}

void BistThread::_p_measurements() {
    // not done yet...
    printf("bms.buck_temp = %d deg C\r\n", (int)global_bms_data.buck_temp);
    printf("bms.mc_cap_voltage = %dmV\r\n", (int)(global_bms_data.mc_cap_voltage*1000));
    printf("bms.contactor_voltage = %dmV\r\n", (int)(global_bms_data.contactor_voltage*1000));
    printf("bms.bms_current = %dmA\r\n", (int)(global_bms_data.bms_current));
    printf("bms.battery.voltage = %dmV\r\n", (int)(global_bms_data.battery.voltage*1000));
    printf("bms.batter.current = %dmA\r\n", (int)(global_bms_data.battery.current*1000));
}

