#include <math.h>
#include "ltc6813.h"
#include "bms_entry.hpp"
#include "threads.hpp"
#include "main.h"

RTOSThread Ltc6813Thread::thread;
bool Ltc6813Thread::enable_balancing;

void Ltc6813Thread::enableBalancing()  { Ltc6813Thread::enable_balancing = 1; }
void Ltc6813Thread::disableBalancing() { Ltc6813Thread::enable_balancing = 0; }

void Ltc6813Thread::initialize() {
    thread = RTOSThread(
        "ltc6813_thread",
        1024*2,
        LTC6813_THREAD_PRIORITY,
        runDriver
    );
}

bool Ltc6813Thread::cell_needs_balancing(uint8_t cell_num) {
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        if (
            ((global_bms_data.battery.cells[cell_num].voltage - global_bms_data.battery.cells[i].voltage) > BALANCING_THRESHOLD) &&
            (cell_num != i)
        ) {
            return true;
        }
    }
    return false;
}

void Ltc6813Thread::runDriver(void* args) {
    // wait for the RTOS and HW timers to stabalize...
    osDelay(5);

    // only need to wake from sleep once, since the periodicity of
    // this thread is less than the sleep timeout time...
    Ltc6813_wakeup_sleep(&ltc6813);

    uint32_t discharge_msk = 0;
    while (1) {
        // TIMING_GPIO_Port->ODR |= TIMING_Pin;

        Ltc6813_wakeup_idle(&ltc6813);

        if (Ltc6813_read_adc(&ltc6813, NORMAL_ADC)) {
            for (uint8_t i = 0; i < NUM_CELLS; i++) {
                global_bms_data.battery.cells[i].voltage = ltc6813.cell_voltages[i];
            }
        }

        if (Ltc6813_read_temp(&ltc6813)) {
            for (uint8_t i = 0; i < NUM_CELLS; i += 2) {
                // each thermistor covers two cells
                global_bms_data.battery.cells[i].temp = ltc6813.thermistor_temps[i/2];
                global_bms_data.battery.cells[i + 1].temp = ltc6813.thermistor_temps[i/2];
            }
        }

        if (Ltc6813Thread::enable_balancing) {
            for (uint8_t i = 0; i < NUM_CELLS; i++) {
                if (Ltc6813Thread::cell_needs_balancing(i)) {
                    discharge_msk |= (1 << i);
                }
                else {
                    discharge_msk &= ~(1 << i);
                }
            }
            Ltc6813_discharge_ctrl(&ltc6813, discharge_msk);
        }

        // TIMING_GPIO_Port->ODR &= ~(TIMING_Pin);
        osDelay(LTC6813_THREAD_PERIODICITY);
    }
}
