#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "coulomb_counting_thread.hpp"
#include "bms_entry.hpp"
#include "threads.hpp"

#define NOMINAL_CAP     21600
#define INIT_SOC        100

RTOSThread CoulombCountingThread::thread;
float CoulombCountingThread::prev_current;

void CoulombCountingThread::initialize() {
    thread = RTOSThread(
        "coulomb_counting_thread",
        1024,
        osPriorityBelowNormal,
        runCoulombCounting
    );
}

float CoulombCountingThread::getCharge() {
    // integrate wrt time to get charge
    float curr_current = global_bms_data.battery.current;
    float trapArea = 0.5 * MEASUREMENT_PERIODICITY * (prev_current + curr_current);
    prev_current = curr_current;
    
    return trapArea;
}

void CoulombCountingThread::runCoulombCounting(void* args) {
    float totalChargeConsumed = 0.0;

    while (1) {
        // osThreadFlagsWait(0x00000001U, osFlagsWaitAll, 0U);
    
        totalChargeConsumed += getCharge();
        float curr_soc = (INIT_SOC - ( (totalChargeConsumed / NOMINAL_CAP)*100 ) );

        if ( (0 < curr_soc) && (curr_soc <= 100) ) {
            global_bms_data.battery.soc = curr_soc;
        }
        else if (curr_soc < 0) {
            global_bms_data.battery.soc = 0;
            totalChargeConsumed = NOMINAL_CAP;
        }
        else if (curr_soc > 100) {
            global_bms_data.battery.soc = 100;
            totalChargeConsumed = 0;
        }
        osDelay(100);
    }
}
