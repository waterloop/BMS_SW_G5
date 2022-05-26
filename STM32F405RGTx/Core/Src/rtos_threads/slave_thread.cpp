#include "bms_entry.hpp"
#include "threads.hpp"
#include "main.h"
#include "can.h"
#include "circ_queue.h"

RTOSThread SlaveThread::thread;
static Queue _rx_queue;
static uint8_t _rx_bytes[sizeof(SlavePkt)];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    SlavePkt* p_pkt = (SlavePkt*)_rx_bytes;
    Queue_put(&_rx_queue, p_pkt);

    HAL_UART_Receive_IT(&huart1, _rx_bytes, sizeof(SlavePkt));
}

void SlaveThread::initialize() {
    thread = RTOSThread(
        "slave_thread",
        1024*2,
        SLAVE_THREAD_PRIORITY,
        runThread
    );
}


void SlaveThread::runThread(void* args) {
    SlavePkt rqst_pkt = {
        .header = SLAVE_PKT_HEADER,
        .addr = 0,
        .payload = {0, 0, 0, 0}
    };
    SlavePkt pkt;

    HAL_UART_Receive_IT(&huart1, _rx_bytes, sizeof(SlavePkt));

    while (1) {
        // Send a request to the slave for data packets
        HAL_UART_Transmit(&huart1, (uint8_t*)&rqst_pkt, sizeof(SlavePkt), 100);

        while (!Queue_empty(&_rx_queue)) {
            // Parse a received message
            Queue_get(&_rx_queue, &pkt);
            if (pkt.addr <= 12) {
                // We are dealing with one of the 12 (2 batt * 6 cell/batt) LiPo cells
                global_bms_data.battery.cells[pkt.addr].voltage = UINT_TO_FLOAT(*((uint32_t*)pkt.payload));
            } else if (pkt.addr <= 14) {
                // We are dealing with one of the temp sensors. We will store them in the first two cell temperatures
                global_bms_data.battery.cells[pkt.addr - 13].temp = UINT_TO_FLOAT(*((uint32_t*)pkt.payload));
            } else {} // Something went wrong, idk maybe do something?
        }

        osDelay(SLAVE_THREAD_PERIODICITY);
    }
}
