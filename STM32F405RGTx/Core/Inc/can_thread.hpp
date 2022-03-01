#include "util.hpp"
#include "can.h"

class CANThread {
    public:
        static void initialize();

        // Requested state change
        static uint8_t stateIdChangeFlag;

        // The device requesting a bus test 
        static uint8_t busTestReqFlag;
    private:

        static RTOSThread thread;
        static void runCANThread(void *args);

        static CANFrame* currentRxFrame;
};