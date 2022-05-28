#include <stdio.h>
#include <stdint.h>
#include "measurements_thread.hpp"
#include "bms_entry.hpp"
#include "cmsis_os.h"
#include "lut.h"
#include "threads.hpp"

#define CURRENT_SENSE_RESISTANCE    1E-3
#define BUCK_SENSE_RESISTANCE       15E-3
#define ADC_TO_VOLTAGE(x) ( (x)*(3.3/((1 << 12) - 1)) )
#define INA240_UNBIAS(v) ( ((v) - (3.3/2))*(1/50) )
#define VOLTAGE_TO_CURRENT(v) (INA240_UNBIAS((v))/CURRENT_SENSE_RESISTANCE)
#define UN_VOLTAGE_DIVIDE(v) ( (21*(v)) )
#define INA180_UNDO_GAIN(v) ((v)/100)
#define INA180_VOLTAGE_TO_CURRENT(v) ( INA180_UNDO_GAIN((v))/BUCK_SENSE_RESISTANCE )

RTOSThread MeasurementsThread::thread;
u_int16_t MeasurementsThread::ADC_buffer[];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    HAL_StatusTypeDef status = HAL_ADC_Stop_DMA(hadc);
    if (status != HAL_OK) {
        printf("Error: HAL_ADC_Stop_DMA failed with status code %d\r\n", status);
        Error_Handler();
    }
    // set flag to signal that ADC conversion has completed
    osThreadFlagsSet(MeasurementsThread::getThreadId(), 0x1U);
}

void MeasurementsThread::initialize() {
    thread = RTOSThread(
        "measurements_thread",
        1024*5,
        MEASUREMENTS_THREAD_PRIORITY,
        runMeasurements
    );
}

void MeasurementsThread::processData() {
    for (uint8_t i = 0; i < ADC_NUM_CONVERSIONS; i++) {
        uint32_t sum = 0;
        for (uint16_t j = 0; j < ADC_DECIMATION_COEFF; j++) {
            sum += ADC_buffer[i + ADC_NUM_CONVERSIONS*j];
        }
        uint32_t val = (sum/ADC_DECIMATION_COEFF);

        switch (i) {
            case 0:
                global_bms_data.buck_temp = ADC_TO_TEMP_LUT[val];
                break;
            case 1:
                global_bms_data.battery.current = VOLTAGE_TO_CURRENT(ADC_TO_VOLTAGE(val));
                break;
            case 2:
                global_bms_data.battery.voltage = UN_VOLTAGE_DIVIDE(ADC_TO_VOLTAGE(val));
                break;
            case 3:
                global_bms_data.mc_cap_voltage = UN_VOLTAGE_DIVIDE(ADC_TO_VOLTAGE(val));
                break;
            case 4:
                global_bms_data.contactor_voltage = UN_VOLTAGE_DIVIDE(ADC_TO_VOLTAGE(val));
                break;
            case 5:
                global_bms_data.bms_current = INA180_VOLTAGE_TO_CURRENT(ADC_TO_VOLTAGE(val))/2;
                break;
        }
    }
}    

void MeasurementsThread::startADCandDMA() {
    // lock kernel to prevent a context switch while starting the DMA
    osKernelLock();

    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(
        &hadc1, (uint32_t*)ADC_buffer, ADC_NUM_CONVERSIONS*ADC_DECIMATION_COEFF);

    if (status != HAL_OK) {
        printf("Error: HAL_ADC_Start_DMA failed with status code %d\r\n", status);
    }
    osKernelUnlock();
}

void MeasurementsThread::runMeasurements(void* args) {
    startADCandDMA();

    while (1) {
        // TIMING_GPIO_Port->ODR |= TIMING_Pin;

        // see if the DMA buffer has been filled
        uint32_t status = osThreadFlagsWait(0x1U, osFlagsWaitAll, 0U);     // 0U for no timeout

        // if the DMA buffer has been filled, process the data...
        if ( (status != osFlagsErrorTimeout) && (status != osFlagsErrorResource) ) {
            processData();

            // set flag to signal to coulomb counting that a new current sample is available...
            osThreadFlagsSet(CoulombCountingThread::getThreadId(), 0x1U);

            // take another measurement
            startADCandDMA();
        }

        // TIMING_GPIO_Port->ODR &= ~(TIMING_Pin);
        osDelay(MEASUREMENTS_THREAD_PERIODICITY);
    }
}

void MeasurementsThread::stopMeasurements() {
    osThreadSuspend(getThreadId());
}

void MeasurementsThread::resumeMeasurements() {
    osThreadResume(getThreadId());
}

osThreadId_t MeasurementsThread::getThreadId() {
    return thread.getThreadId();
}
