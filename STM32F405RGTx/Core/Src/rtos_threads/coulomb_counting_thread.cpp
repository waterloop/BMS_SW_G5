#include "main.h"
#include "cmsis_os.h"
#include "coulomb_counting_thread.hpp"
#include "bms_entry.hpp"
#include "threads.hpp"

RTOSThread CoulombCountingThread::thread;
float CoulombCountingThread::prev_current;
uint32_t CoulombCountingThread::prev_time;

osThreadId_t CoulombCountingThread::getThreadId() { return CoulombCountingThread::thread.getThreadId(); }

void CoulombCountingThread::initialize() {
    thread = RTOSThread(
        "coulomb_counting_thread",
        1024,
        COULOMB_COUNTING_THREAD_PRIORITY,
        runCoulombCounting
    );
}

float CoulombCountingThread::getCharge() {
    // integrate wrt time to get charge
    float curr_current = global_bms_data.battery.current;
    uint32_t curr_time = HAL_GetTick();

    float elapsed_time = (curr_time - CoulombCountingThread::prev_time)*1E-3;

    float trapArea = 0.5*elapsed_time*(prev_current + curr_current);

    CoulombCountingThread::prev_current = curr_current;
    CoulombCountingThread::prev_time = curr_time;

    return trapArea;
}

void CoulombCountingThread::runCoulombCounting(void* args) {
    float totalChargeConsumed = 0.0;
    prev_time = HAL_GetTick();

    while (1) {
        uint32_t status = osThreadFlagsWait(0x00000001U, osFlagsWaitAll, 0U);

        if ( (status != osFlagsErrorTimeout) && (status != osFlagsErrorResource) ) {
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
        }

        osDelay(COULOMB_COUNTING_THREAD_PERIODICITY);
    }
}
