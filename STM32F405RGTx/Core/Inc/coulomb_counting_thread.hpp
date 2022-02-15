#pragma once 
#include "util.hpp"

class CoulombCountingThread {
    public:
        static void initialize();

        static float getCharge();
    private:
        static RTOSThread thread;
        static void runCoulombCounting(void* args);

        static float prev_current;
};