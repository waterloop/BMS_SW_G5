#pragma once

#include "util.hpp"

class LEDThread {
    public:
        static void initialize();
        static void setLED(float R_, float G_, float B_, uint8_t blink_);

    private:
        static RTOSThread thread_;
        static uint32_t cnt;

        static float R;
        static float G;
        static float B;
        static uint8_t rgb_blink;
        static uint8_t rgb_on;

    private:
        static void runLEDThread(void* args);
};

