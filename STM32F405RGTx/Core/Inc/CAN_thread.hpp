#pragma once

#include "util.hpp"
#include "can.h"

#define BUS_TEST_REQ_TIMEOUT 10000

extern osMutexId_t g_bus_mutex;

HAL_StatusTypeDef send_frame(CANFrame* frame);

class CANThread {
    public:
        static void initialize();

    private:
        static RTOSThread thread_;
        static void runCANThread(void* arg);

    private:
        static void send_heartbeat();
};


