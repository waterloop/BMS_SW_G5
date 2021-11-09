#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "threads.h"

#define OV_BOUNDS 3.7    // upper bounds of cell voltage -- overvoltage limit
#define UV_BOUNDS 2.7    //  lower bounds of cell voltage -- undervoltage limit
#define TARGET_VOLTAGE 3.6   // target cell voltage

void ovCheck () {

    uint32_t dischargeCells = 0;

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        float cellVoltage = global_bms_data.cells[n].voltage;

        if (cellVoltage > OV_BOUNDS)
        {
            discargeCells |= (1 << n);
        }

    }

    ovBalance(dischargeCells);

}

void uvCheck () {

    uint32_t dischargeCells = 0;

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        float cellVoltage = global_bms_data.cells[n].voltage;

        if (cellVoltage < UV_BOUNDS)
        {
            for (uint8_t j = 0; j < NUM_CELLS; j++)
            {
                if (j == n)
                {
                    continue;
                }

                else if (global_bms_data.cells[j].voltage <= cellVoltage)
                {
                    dischargeCells &= ~(1 << j);
                    continue;
                }

                dischargeCells |= (1 << j);
            }

        }
    }

}

void ovBalance (uint32_t overchargedCells) {

    float ovBalanceVoltage = TARGET_VOLTAGE + 10E-3;    // tolerance of 10mV added

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        uint8_t nthCellStatus = (overchargedCell >> n) & 1;

        if (nthCellStatus == 1)
        {
            ltc6813_discharge_ctrl(&ltc6813, (1 << n));
        }
    }

    for (uint8_t i = 0 ; i < NUM_CELLS ; i++)
    {

        uint8_t nCellStatus = (overchargedCell >> n) & 1;

        if (nCellStatus == 0)
        {
            continue;
        }

        float checkVoltage = global_bms_data.cells[i].voltage;

        if (checkVoltage <= ovBalanceVoltage)
        {
            ltc6813_discharge_ctrl(&ltc6813, &= ~(0 << j));
            dischargeCells &= ~(1 << j)
        }

    } // needs to be encapsulated by while loop

    // figure out how to stop discharging individual cells once they reach target voltage

}

void uvBalance (uint32_t overchargedCells) {

    float uvTargetVoltage;

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        uint8_t nthCellStatus = (overchargedCell >> n) & 1;

        if (nthCellStatus == 1)
        {
            ltc6813_discharge_ctrl(&ltc6813, (1 << n));
        }

        else if (nthCellStatus == 0)
        {
            uvTargetVoltage = global_bms_data.cells[n].voltage;
        }
    }

    for (uint8_t i = 0 ; i < NUM_CELLS ; i++)
    {

        uint8_t nCellStatus = (overchargedCell >> n) & 1;

        if (nCellStatus == 0)
        {
            continue;
        }

        float checkVoltage = global_bms_data.cells[i].voltage;

        if (checkVoltage <= uvTargetVoltage)
        {
            ltc6813_discharge_ctrl(&ltc6813, (1 << j));
        }

    } // needs to be encapsulated by while loop

    // figure out how to stop discharging individual cells once they reach target voltage

}

/*void overchargeBalance (int imbalancedCell) {   // OV fxn, discharges imbalanced cell to target voltage

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

}*/