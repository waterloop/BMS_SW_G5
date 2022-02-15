#pragma once

#define NUM_CELLS   14

typedef struct {
    float voltage;
    float temp;
} Cell;

typedef struct {
    float voltage;
    float current;
    float soc;
    Cell cells[NUM_CELLS];
} Battery;

typedef struct {
    float buck_temp;
    float mc_cap_voltage;
    float contactor_voltage;
    float shunt_current;
    Battery battery;
} BMS;

int bms_entry();
void __report_CAN();
void __cell_disable();
void __hard_fault_state_trans();

