#pragma once

#include "util.hpp"

class DebugLEDThread {
    public:
        static void initialize();

    private:
        static RTOSThread thread_;
        static void runDebugLEDThread(void* args);
};

