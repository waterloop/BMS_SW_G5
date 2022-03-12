#include "main.h"
#include "threads.hpp"
#include <debug_led_thread.hpp>

RTOSThread DebugLEDThread::thread_;

void DebugLEDThread::initialize() {
    DebugLEDThread::thread_ = RTOSThread(
        "debug_led_thread",
        200,
        osPriorityIdle,
        DebugLEDThread::runDebugLEDThread
    );
}

void DebugLEDThread::runDebugLEDThread(void* args) {
    while (1) {
        DEBUG_GPIO_Port->ODR ^= DEBUG_Pin;
        osDelay(1000);
    }
}

//    GPIOB->MODER &= ~(0b11 << (3*2));
//    GPIOB->MODER |= (0b01 << (3*2));

