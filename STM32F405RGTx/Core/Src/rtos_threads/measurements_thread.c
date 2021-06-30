#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "cmsis_os.h"
#include "ltc6813.h"
#include "threads.h"

#define ADC_NUM_CONVERSIONS			5U
#define ADC_DECIMATION_COEFF		256U
#define ADC_TO_VOLTAGE_COEFF		(3.3/(1 << 12))

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
	osThreadFlagsSet(measurements_thread, 0x00000001U);		// set flag to signal that ADC conversion has completed
}

void _adc_decimation() {
	for (uint8_t i = 0; i < ADC_NUM_CONVERSIONS; i++) {
		uint32_t sum = 0;
		for (uint16_t j = 0; j < ADC_DECIMATION_COEFF; j++) {
			sum += ADC_buffer[i + ADC_NUM_CONVERSIONS*j];
		}
		float voltage = (sum/ADC_DECIMATION_COEFF)*ADC_TO_VOLTAGE_COEFF;

		if (i == 0) { global_bms_data.buck_temp = voltage; }
		else if (i == 1) { global_bms_data.battery.current = voltage; }
		else if (i == 2) { global_bms_data.battery.voltage = voltage; }
		else if (i == 3) { global_bms_data.mc_cap_voltage = voltage; }
		else if (i == 4) { global_bms_data.contactor_voltage = voltage; }
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
		osKernelUnlock();
		// wait for signal from HAL_ADC_ConvCpltCallback and give execution over to over threads
		osThreadFlagsWait(0x00000001U, osFlagsWaitAll, 0U);		// 0U for no timeout
		_adc_decimation();

		// LTC6813 COMMANDS
		Ltc6813_wakeup_sleep(&ltc6813);

		printf("CFG A\r\n");
		Ltc6813_wakeup_idle(&ltc6813);
		if ( Ltc6813_read_cfga(&ltc6813) ) { printf("PEC SUCCESS\r\n"); }
		else { printf("PEC FAIL\r\n"); }
		Buffer_print( &(ltc6813.cfga_bfr) );

		printf("CFG B\r\n");
		Ltc6813_wakeup_idle(&ltc6813);
		if ( Ltc6813_read_cfgb(&ltc6813) ) { printf("PEC SUCCESS\r\n"); }
		else { printf("PEC FAIL\r\n"); }
		Buffer_print( &(ltc6813.cfgb_bfr) );

		osDelay(200);
	}
}

