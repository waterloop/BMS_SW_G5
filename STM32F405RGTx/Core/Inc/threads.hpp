#pragma once

#include "cmsis_os.h"
#include "coulomb_counting_thread.hpp"
#include "measurements_thread.hpp"
#include "state_machine.hpp"
#include "ltc6813_thread.hpp"
#include "bist_thread.hpp"
#include "debug_led_thread.hpp"

// In milliseconds
#define MEASUREMENT_PERIODICITY         100
#define LTC_MEASUREMENT_PERIODICITY     500
#define BIST_PERIODICITY                50
#define COULOMB_COUNTING_PERIODICITY    150
#define DEBUG_LED_PERIODICITY           1000
#define STATE_MACHINE_PERIODICITY       200
