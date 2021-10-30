#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "cmsis_os.h"
#include "lut.h"
#include "ltc6813.h"
#include "threads.h"

#define ADC_NUM_CONVERSIONS			5U
#define ADC_DECIMATION_COEFF		256U

#define CURRENT_SENSE_RESISTANCE	1E-3

#define ADC_TO_VOLTAGE(x) ( x*(3.3/(1 << 12)) )

#define INA240_UNBIAS(v) ( (v - (3.3/2))*(1/50) )
#define VOLTAGE_TO_CURRENT(v) (INA240_UNBIAS(v)/CURRENT_SENSE_RESISTANCE)

#define UN_VOLTAGE_DIVIDE(v) ( (21*v) )

const osThreadAttr_t measurements_thread_attrs = {
    .name = "measurements_thread",
    .priority = (osPriority_t)osPriorityAboveNormal,
    .stack_size = 1024*5
};

uint16_t ADC_buffer[ADC_NUM_CONVERSIONS*ADC_DECIMATION_COEFF];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    HAL_StatusTypeDef status = HAL_ADC_Stop_DMA(hadc);
    if (status != HAL_OK) {
        printf("Error: HAL_ADC_Stop_DMA failed with status code %d\r\n", status);
        Error_Handler();
    }
    osThreadFlagsSet(measurements_thread, 0x00000001U);        // set flag to signal that ADC conversion has completed
}

void _adc_decimation() {
    for (uint8_t i = 0; i < ADC_NUM_CONVERSIONS; i++) {
        uint32_t sum = 0;
        for (uint16_t j = 0; j < ADC_DECIMATION_COEFF; j++) {
            sum += ADC_buffer[i + ADC_NUM_CONVERSIONS*j];
        }
        uint32_t val = (sum/ADC_DECIMATION_COEFF);

        switch (i) {
            case 0:
                global_bms_data.buck_temp = ADC_TO_TEMP_LUT[val];
            case 1:
                global_bms_data.battery.current = VOLTAGE_TO_CURRENT(ADC_TO_VOLTAGE(val));
			case 2:
				global_bms_data.battery.voltage = UN_VOLTAGE_DIVIDE(ADC_TO_VOLTAGE(val));
			case 3:
				global_bms_data.mc_cap_voltage = UN_VOLTAGE_DIVIDE(ADC_TO_VOLTAGE(val));
			case 4:
				global_bms_data.contactor_voltage = UN_VOLTAGE_DIVIDE(ADC_TO_VOLTAGE(val));
        }
    }
}

void measurements_thread_fn(void* arg) {
    while (1) {
        // MISC ADC READINGS
        osKernelLock();
        HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_buffer, ADC_NUM_CONVERSIONS*ADC_DECIMATION_COEFF);
        if (status != HAL_OK) {
            printf("Error: HAL_ADC_Start_DMA failed with status code %d\r\n", status);
        }

        // // LTC6813 COMMANDS
//        osKernelLock();

        Ltc6813_wakeup_sleep(&ltc6813);

        Ltc6813_wakeup_idle(&ltc6813);
        Ltc6813_read_adc(&ltc6813, NORMAL_ADC);
        Ltc6813_print_voltages(self);

        osKernelUnlock();
        // wait for signal from HAL_ADC_ConvCpltCallback and give execution over to other threads
        osThreadFlagsWait(0x00000001U, osFlagsWaitAll, 0U);        // 0U for no timeout
        _adc_decimation();


//         printf("CFG A\r\n");
//         Ltc6813_wakeup_idle(&ltc6813);
//         if ( Ltc6813_read_cfga(&ltc6813) ) { printf("PEC SUCCESS\r\n"); }
//         else { printf("PEC FAIL\r\n"); }
//         Buffer_print( &(ltc6813.cfga_bfr) );
//
//         printf("CFG B\r\n");
//         Ltc6813_wakeup_idle(&ltc6813);
//         if ( Ltc6813_read_cfgb(&ltc6813) ) { printf("PEC SUCCESS\r\n"); }
//         else { printf("PEC FAIL\r\n"); }
//         Buffer_print( &(ltc6813.cfgb_bfr) );

//        osDelay(MEASUREMENT_PERIODICITY*1E3);

    }
}

