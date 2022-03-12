#pragma once

#define NUM_CELLS   14

class Cell {
    public:
        float voltage;
        float temp;
};

class Battery {
    public:
        float voltage;
        float current;
        float soc;
        Cell cells[NUM_CELLS];
};

class BMS {
    public:
        float buck_temp;
        float mc_cap_voltage;
        float contactor_voltage;
        float bms_current;
        Battery battery;

        // Helper function for tests w/o HV battery
        void _lv_test_init();
};

int bms_entry();

extern BMS global_bms_data;
void _report_CAN();
void _cell_disable();
void _hard_fault_state_trans();
void _set_fault_checking(bool val);

