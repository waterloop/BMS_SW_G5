#pragma once
#include "util.hpp"

class MeasurementsThread {
    public:
        static void initialize();

        static void _process_data();
    private:
        static RTOSThread thread;
        static void runMeasurements(void* args);

        static uint16_t ADC_buffer;
        
        static void _start_adc_and_dma();

        static void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
};