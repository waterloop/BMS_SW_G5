#ifndef __THREADS
#define __THREADS

#include "cmsis_os.h"

#define MEASUREMENT_PERIODICITY     100E-3

osThreadId_t ext_led_blink_thread;
const osThreadAttr_t ext_led_blink_thread_attrs;
void ext_led_blink_thread_fn(void* arg);

osThreadId_t measurements_thread;
const osThreadAttr_t measurements_thread_attrs;
void measurements_thread_fn(void* arg);

osThreadId_t fsm_thread;
const osThreadAttr_t fsm_thread_attrs;
void fsm_thread_fn(void* arg);

osThreadId_t coulomb_counting_thread;
const osThreadAttr_t coulomb_counting_thread_attrs;
void coulomb_counting_thread_fn(void* arg);

#endif
