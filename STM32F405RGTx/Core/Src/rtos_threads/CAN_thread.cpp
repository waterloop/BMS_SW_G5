#include <stdio.h>
#include "main.h"
#include "can.h"
#include "bms_entry.hpp"
#include "threads.hpp"
#include "CAN_thread.hpp"

osMutexId_t g_bus_mutex;
osMessageQueueId_t g_state_change_req_queue;

RTOSThread CANThread::thread_;

HAL_StatusTypeDef send_frame(CANFrame* frame) {
    if (osMutexAcquire(g_bus_mutex, 0U) != osOK) { Error_Handler(); }
    HAL_StatusTypeDef ret = CANBus_put_frame(frame);
    if (osMutexRelease(g_bus_mutex) != osOK) { Error_Handler(); }
    return ret;
}

void CANThread::initialize() {
    CANThread::thread_ = RTOSThread(
        "CAN_thread",
        1024,
        CAN_THREAD_PRIORITY,
        CANThread::runCANThread
    );

    osMutexAttr_t g_bus_mutex_attrs = {
        .name = "bus_mutex",
        .attr_bits = osMutexPrioInherit
    };
    g_bus_mutex = osMutexNew(&g_bus_mutex_attrs);
    if (g_bus_mutex == NULL) { Error_Handler(); }

    g_state_change_req_queue = osMessageQueueNew(10, sizeof(StateID), NULL);

    printf("initializing CAN peripheral...\r\n");
    if (CANBus_init(&hcan1, &htim7) != HAL_OK) { Error_Handler(); }
    if (CANBus_subscribe(STATE_CHANGE_REQ) != HAL_OK) { Error_Handler(); }
    if (CANBus_subscribe_mask(BUS_TEST_REQ_BASE, BUS_TEST_MSK) != HAL_OK) { Error_Handler(); }
    if (CANBus_subscribe_mask(BUS_TEST_RESP_BASE, BUS_TEST_MSK) != HAL_OK) { Error_Handler(); }
}

void CANThread::send_heartbeat() {
    float avg_cell_temp = 0;
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        avg_cell_temp += global_bms_data.battery.cells[i].temp;
    }
    avg_cell_temp /= NUM_CELLS;

    CANFrame tx_frame0 = CANFrame_init(BATTERY_PACK_CURRENT.id);
    CANFrame_set_field(&tx_frame0, BATTERY_PACK_CURRENT, FLOAT_TO_UINT(global_bms_data.battery.current));
    CANFrame_set_field(&tx_frame0, CELL_TEMPERATURE, FLOAT_TO_UINT(avg_cell_temp));

    CANFrame tx_frame1 = CANFrame_init(BATTERY_PACK_VOLTAGE.id);
    CANFrame_set_field(&tx_frame1, BATTERY_PACK_VOLTAGE, FLOAT_TO_UINT(global_bms_data.battery.voltage));
    CANFrame_set_field(&tx_frame1, STATE_OF_CHARGE, FLOAT_TO_UINT(global_bms_data.battery.soc));

    CANFrame tx_frame2 = CANFrame_init(BUCK_TEMPERATURE.id);
    CANFrame_set_field(&tx_frame2, BUCK_TEMPERATURE, FLOAT_TO_UINT(global_bms_data.buck_temp));
    // CANFrame_set_field(&tx_frame2, BMS_CURRENT, FLOAT_TO_UINT(0));

    CANFrame tx_frame3 = CANFrame_init(MC_CAP_VOLTAGE.id);
    CANFrame_set_field(&tx_frame3, MC_CAP_VOLTAGE, FLOAT_TO_UINT(global_bms_data.mc_cap_voltage));

    if (osMutexAcquire(g_bus_mutex, 0U) != osOK) { Error_Handler(); }

    if (CANBus_put_frame(&tx_frame0) != HAL_OK) { Error_Handler(); }
    if (CANBus_put_frame(&tx_frame1) != HAL_OK) { Error_Handler(); }
    if (CANBus_put_frame(&tx_frame2) != HAL_OK) { Error_Handler(); }
    if (CANBus_put_frame(&tx_frame3) != HAL_OK) { Error_Handler(); }

    if (osMutexRelease(g_bus_mutex) != osOK) { Error_Handler(); }
}

void CANThread::runCANThread(void* arg) {
    // send a BUS_TEST_REQ, if we don't get a response we are disconnected from the bus...
    CANFrame req_frame = CANFrame_init(MOTOR_CONTROLLER_REQ);
    send_frame(&req_frame);

    // wait for a response...
    uint32_t start_time = HAL_GetTick();
    while (!Queue_empty(&RX_QUEUE)) {
        if ((HAL_GetTick() - start_time) > BUS_TEST_REQ_TIMEOUT) {
            StateMachineThread::setState(NormalDangerFault);
        }
    }

    while (1) {
        if (!Queue_empty(&RX_QUEUE)) {
            CANFrame rx_frame = CANBus_get_frame();
            if (rx_frame.id == STATE_CHANGE_REQ) {
                StateID state_change_req_id = (StateID)CANFrame_get_field(&rx_frame, STATE_ID);
                osMessageQueuePut(g_state_change_req_queue, &state_change_req_id, 0, 0);
                break;
            }
        }

        CANThread::send_heartbeat();
        if (RELAY_HEARTBEAT_ERROR_FLAG) { StateMachineThread::setState(NormalDangerFault); }

        osDelay(CAN_THREAD_PERIODICITY);
    }
}

