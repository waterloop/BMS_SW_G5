#include <stdio.h>

float prev_current = 0.0;

float getCharge() {

    // ADC sample rate, which doubles down as width of individual subdivision (10 MHz)
    const double WIDTH = 0.000001;

    // integrate wrt time to get charge
    float curr_current = global_bms_date.battery.current;
	 
    float trapArea = 0.5 * WIDTH * (prev_current + curr_current);	// how do I deal with this when the BMS starts and only one sample is available
 
 	prev_current = curr_current;
 	
    return totArea;
}

void coulombCountingEntry(void* arg) {
    
    float totalChargeConsumed = 0.0;

    while (1) {
    	
    	osThreadFlagsWait(0x00000001U, osFlagsWaitAll, 0U);
    
        totalChargeConsumed += getCharge();
        // this should be timed so totalChargeConsumed is only compounded when a new value is available
        
       	float curr_soc = INIT_SOC - (totalChargeConsumed / NOMINAL_CAP) * 100;
       	// INIT_SOC is 100, NOMINAL_CAP must be determined. these should use #define
       	
       	// how can SoC be written back to the bms object?
       	global_bms_object.battery.soc = curr_soc;
             
    }
}