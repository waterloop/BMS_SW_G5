#pragma once

#include "cmsis_os.h"
#include "coulomb_counting_thread.hpp"
#include "measurements_thread.hpp"
#include "state_machine.hpp"
#include "ltc6813_thread.hpp"
#include "bist_thread.hpp"
#include "LED_thread.hpp"
#include "slave_thread.hpp"
#include "CAN_thread.hpp"

// see ./docs/utilization.xlsx for schedulability calculations

// Periodicities are in milli-seconds

// tasks with HARD timing deadlines
#define MEASUREMENTS_THREAD_PERIODICITY             3
// #define MEASUREMENTS_THREAD_PERIODICITY             100
#define MEASUREMENTS_THREAD_PRIORITY                osPriorityRealtime7

#define COULOMB_COUNTING_THREAD_PERIODICITY         4
// #define COULOMB_COUNTING_THREAD_PERIODICITY         150
#define COULOMB_COUNTING_THREAD_PRIORITY            osPriorityRealtime6

#define STATE_MACHINE_THREAD_PERIODICITY            7
// #define STATE_MACHINE_THREAD_PERIODICITY            200
#define STATE_MACHINE_THREAD_PRIORITY               osPriorityRealtime5

#define CAN_THREAD_PERIODICITY                      200
#define CAN_THREAD_PRIORITY                         osPriorityRealtime4

#define LTC6813_THREAD_PERIODICITY                  500
#define LTC6813_THREAD_PRIORITY                     osPriorityRealtime3

#define SLAVE_THREAD_PERIODICITY                  500
#define SLAVE_THREAD_PRIORITY                     osPriorityRealtime1


// tasks with SOFT timing deadlines
#define BIST_THREAD_PERIODICITY                     50
#define BIST_THREAD_PRIORITY                        osPriorityIdle

#define LED_THREAD_PERIODICITY                      250
#define LED_THREAD_PRIORITY                         osPriorityLow


