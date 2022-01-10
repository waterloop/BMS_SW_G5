#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "threads.hpp"

#define NOMINAL_CAP     21600
#define INIT_SOC        100

float prev_current = 0.0;

float getCharge() {
    // ADC sample rate, which doubles down as width of individual subdivision (10 MHz)
    const double WIDTH = MEASUREMENT_PERIODICITY;

    // integrate wrt time to get charge
    float curr_current = global_bms_data.battery.current;
    float trapArea = 0.5 * WIDTH * (prev_current + curr_current);
    prev_current = curr_current;
    
    return trapArea;
}

const osThreadAttr_t coulomb_counting_thread_attrs = {
    .name = "coulomb_counting_thread",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityBelowNormal
};

void coulomb_counting_thread_fn(void* arg) {
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
