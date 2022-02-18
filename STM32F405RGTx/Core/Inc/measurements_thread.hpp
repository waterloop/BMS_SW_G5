#pragma once
#include "util.hpp"

class MeasurementsThread {
    public:
        static void initialize();

    private:
        static RTOSThread thread;
        static void runMeasurements(void* args);

        static uint16_t *ADC_buffer;

        static void processData();
        
        static void startADCandDMA();

        static void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
};