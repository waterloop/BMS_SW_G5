#pragma once
#include "util.hpp"

class Ltc6813Thread {
    public:
        static void initialize();

        static osThreadId_t getThreadId();
    private:
        static RTOSThread thread;
        static void runDriver(void* args);
};