#include "ltc6813_thread.hpp"
#include "ltc6813.h"
#include "bms_entry.hpp"
#include "threads.hpp"
#include "main.h"

RTOSThread Ltc6813Thread::thread;

void Ltc6813Thread::initialize() {
    thread = RTOSThread(
        "ltc6813_thread",
        1024*2,
        LTC6813_THREAD_PRIORITY,
        runDriver
    );
}

void Ltc6813Thread::runDriver(void* args) {
    while (1) {
        Ltc6813_wakeup_sleep(&ltc6813);
        Ltc6813_wakeup_idle(&ltc6813);
        if (Ltc6813_read_adc(&ltc6813, NORMAL_ADC)) {
            for (uint8_t i = 0; i < NUM_CELLS; i++) {
                global_bms_data.battery.cells[i].voltage = ltc6813.cell_voltages[i];
            }
        }
        osDelay(LTC6813_THREAD_PERIODICITY);
    }
}
