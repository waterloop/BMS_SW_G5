#include <stdio.h>
#include "main.h"
#include "threads.h"
#include "state_machine.h"
#include "bms_entry.h"
#include "ltc6813.h"
#include "passive_cell_balancing.h"

void ovCheck () {   // this fxn checks for overvoltage of individual cells and writes to the bitmask if it's seen

    uint32_t dischargeCells = 0;

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        float cellVoltage = global_bms_data.battery.cells[n].voltage; // add battery

        if (cellVoltage > OV_BOUNDS)
        {
            dischargeCells |= (1 << n);  // setting given bitmask to 1
        }

    }

    ovBalance(dischargeCells); // sends bitmask to OV balance fxn

}

void uvCheck () {   // this fxn checks for undervoltage of individual cells and writes to the bitmask if it's seen

    uint32_t dischargeCells = 0;

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        float cellVoltage = global_bms_data.battery.cells[n].voltage;

        if (cellVoltage < UV_BOUNDS)
        {
            for (uint8_t j = 0; j < NUM_CELLS; j++)
            {
                if (j == n)
                {
                    continue;
                }

                else if (global_bms_data.battery.cells[j].voltage <= cellVoltage)
                {
                    dischargeCells &= ~(1 << j);
                    continue;
                }

                dischargeCells |= (1 << j);
            }

        }
    }

}

void ovBalance (uint32_t overchargedCells) {    // this fxn should enable and disable discharging of individual cells

    float ovBalanceVoltage = TARGET_VOLTAGE + 10E-3;    // tolerance of 10mV added

    for (uint8_t n = 0 ; n < NUM_CELLS ; n++)
    {
        uint8_t nthCellStatus = (overchargedCells >> n) & 1;

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

        float checkVoltage = global_bms_data.battery.cells[i].voltage;

        if (checkVoltage <= ovBalanceVoltage)
        {
            ltc6813_discharge_ctrl(&ltc6813, &= ~(0 << j));
        }

    } // needs to be encapsulated by while loop

    // figure out how to stop discharging individual cells once they reach target voltage

}

void uvBalance (uint32_t overchargedCells) { // this fxn should enable and disable discharging of individual cells

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
            uvTargetVoltage = global_bms_data.battery.cells[n].voltage;
        }
    }

    for (uint8_t i = 0 ; i < NUM_CELLS ; i++)
    {

        uint8_t nCellStatus = (overchargedCell >> n) & 1;

        if (nCellStatus == 0)
        {
            continue;
        }

        float checkVoltage = global_bms_data.battery.cells[i].voltage;

        if (checkVoltage <= uvTargetVoltage)
        {
            ltc6813_discharge_ctrl(&ltc6813, (1 << j));
        }

    } // needs to be encapsulated by while loop

    // figure out how to stop discharging individual cells once they reach target voltage

}