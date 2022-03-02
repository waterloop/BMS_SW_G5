#include "util.hpp"
#include "can.h"

class CANThread {
    public:
        static void initialize();

        static uint8_t hasStateReq();
        // static uint8_t hasBusTestReq();

        static StateID getStateChange();
        // static uint8_t getBusTestReq();

    private:
        static RTOSThread thread;
        static void runCANThread(void *args);

        static CANFrame* currentRxFrame;

        // Requested state change
        static StateID stateChangeReq;

        // The device requesting a bus test 
        // static uint8_t busTestReq;
};