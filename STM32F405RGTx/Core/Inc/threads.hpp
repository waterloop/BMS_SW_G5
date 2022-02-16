#pragma once

#include "cmsis_os.h"
#include "coulomb_counting_thread.hpp"
#include "state_machine.hpp"

#define MEASUREMENT_PERIODICITY     100E-3

extern osThreadId_t measurements_thread;
extern const osThreadAttr_t measurements_thread_attrs;
void measurements_thread_fn(void* arg);