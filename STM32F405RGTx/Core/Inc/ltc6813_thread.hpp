#pragma once
#include "util.hpp"

// here as a placeholder
#define BALANCING_THRESHOLD     0.2 // Volts

class Ltc6813Thread {
    public:
        static void initialize();
        static osThreadId_t getThreadId();

        static void enableBalancing();
        static void disableBalancing();

    private:
        static RTOSThread thread;
        static void runDriver(void* args);

    private:
        static bool enable_balancing;
        static bool cell_needs_balancing(uint8_t cell_num);
};
