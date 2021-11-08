#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "threads.h"

const int NUM_CELLS = 14;   // number of cells in pack
const float OV_BOUNDS = 3.7;    // upper bounds of cell voltage -- overvoltage limit
const float UV_BOUNDS = 2.7;    //  lower bounds of cell voltage -- undervoltage limit
const float TARGET_VOLTAGE = 3.6;   // target cell voltage

void cellCheck () { // this fxn checks each cell for under/overvoltage and calls the respective balancing fxn

    for (int n = 0 ; n < NUM_CELLS ; n++)
    {
        float cellVoltage = global_bms_data.cells[n].voltage;

        if (cellVoltage > OV_BOUNDS)
            overchargeBalance(n);

        else if (cellVoltage < UV_BOUNDS)
            underchargeBalance(n);

    }

}

void overchargeBalance (int imbalancedCell) {   // OV fxn, discharges imbalanced cell to target voltage

    float balanceVoltage = TARGET_VOLTAGE + 0.01; // tolerance of 10mV added

    do  {
        ltc6813_discharge_ctrl(&ltc6813, (1 << imbalancedCell));
    } while (global_bms_data.cells[imbalancedCell].voltage > balanceVoltage);

    ltc6813_discharge_ctrl(&ltc6813, (0 << imbalancedCell));

}


void underchargeBalance (int imbalancedCell) {  // UV fxn, discharges all other cells to imbalanced cell

    float balanceVoltage = global_bms_data.cells[imbalancedCell].voltage + 0.01;

    do {

        for (int i = 0; i < NUM_CELLS; i++)
        {

            if (i == imbalancedCell)    // skips imbalanced cell in loop
                continue;

            float cellVoltage = global_bms_data.cells[i].voltage;

            if (cellVoltage > balanceVoltage)   // only discharge if V lower than that of target
                ltc6813_discharge_ctrl(&ltc6813, (1 << imbalancedCell));

            for (int j = 0; j < NUM_CELLS; j++) // this loop checks for cells that have already been discharged enough
            {
                if (j == imbalancedCell)
                    continue;

                float checkVoltage = global_bms_data.cells[j].voltage

                if (checkVoltage <= balanceVoltage) // toggle off discharge for cells that are done discharging
                    ltc6813_discharge_ctrl(&ltc6813, (0 << j));
            }

        }

    } while (global_bms_data.battery.voltage > balanceVoltage * NUM_CELLS);
        // balance only while voltage of cells is greater than target voltage

}