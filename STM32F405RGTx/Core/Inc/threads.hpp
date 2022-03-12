#pragma once

#include "cmsis_os.h"
#include "coulomb_counting_thread.hpp"
#include "measurements_thread.hpp"
#include "state_machine.hpp"
#include "ltc6813_thread.hpp"
#include "bist_thread.hpp"
#include "debug_led_thread.hpp"

// In seconds
#define MEASUREMENT_PERIODICITY     100E-3
#define LTC_MEASUREMENT_PERIODICITY 500E-3
