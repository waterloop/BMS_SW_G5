#include "can_thread.hpp"
#include "can.h"

CANFrame* CANThread::currentRxFrame;

void CANThread::initialize() {
    thread = RTOSThread(
        "can_thread",
        1024,
        osPriorityHigh,
        runCANThread
    );

    stateIdChangeFlag = -1;
    busTestReqFlag = -1;
}

void CANThread::runCANThread(void *args) {
    while (1) {
        // Load next queue message if our current message has been dealt with
        if (!Queue_empty(&RX_QUEUE) && !currentRxFrame) {
            CANFrame returnFrame = CANBus_get_frame();
            currentRxFrame = &returnFrame;
        }

        if (currentRxFrame) {
            // If we have a message, let's handle it
            if (currentRxFrame->id == STATE_CHANGE_REQ && stateIdChangeFlag < 0) {
                stateIdChangeFlag = CANFrame_get_field(currentRxFrame, STATE_ID);
                currentRxFrame = NULL;
            }  
            // else if (currentRxFrame->id >= BUS_TEST_REQ_BASE && currentRxFrame->id <= BUS_TEST_REQ_BASE | 0xF && busTestReqFlag < 0) {
            //     busTestReqFlag = currentRxFrame->id - BUS_TEST_REQ_BASE;
            //     currentRxFrame = NULL;
            // }
        }

        osDelay(20);
    }
}