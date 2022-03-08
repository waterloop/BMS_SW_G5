#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "threads.hpp"
#include "bms_entry.hpp"
#include "bist_thread.hpp"

void BistThread::initialize() {
    BistThread::thread_ = RTOSThread(
            "bist_thread",
            1024,
            osPriorityLow,
            BistThread::runBist
    );
}

void BistThread::runBist(void* args) {
    while (1) {

    }
}

static void _sinput(char* prompt, char* buff, uint32_t* len) {
    printf(prompt);
    char curr_char;
    uint32_t curr_len = 0;

    while (curr_len < (*len)) {
        uint8_t tmp;

        // try to receive 1 character with a timeout value of 1ms
        HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, &tmp, 1, 1);

        if (status == HAL_TIMEOUT) {
            continue;
        }
        else if (status == HAL_OK) {
            if (tmp == "\n") {
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
    }
    *len = curr_len;
}

