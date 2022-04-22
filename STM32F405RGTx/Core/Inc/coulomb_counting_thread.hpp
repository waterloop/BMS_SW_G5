#pragma once 
#include "util.hpp"

#define NOMINAL_CAP     21600
#define INIT_SOC        100

class CoulombCountingThread {
    public:
        static void initialize();
        static osThreadId_t getThreadId();

    private:
        static RTOSThread thread;
        static void runCoulombCounting(void* args);
        static float getCharge();

        static float prev_current;
        static uint32_t prev_time;
};
