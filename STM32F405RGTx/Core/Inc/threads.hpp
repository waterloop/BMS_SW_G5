#pragma once

#include "cmsis_os.h"

#define MEASUREMENT_PERIODICITY     100E-3

extern osThreadId_t measurements_thread;
extern const osThreadAttr_t measurements_thread_attrs;
void measurements_thread_fn(void* arg);

extern osThreadId_t coulomb_counting_thread;
extern const osThreadAttr_t coulomb_counting_thread_attrs;
void coulomb_counting_thread_fn(void* arg);

extern osThreadId_t state_machine_thread;
extern const osThreadAttr_t state_machine_thread_attrs;
void StartStateMachine(void* arg);
