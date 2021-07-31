#ifndef __THREADS
#define __THREADS

#include "cmsis_os.h"

osThreadId_t ext_led_blink_thread;
const osThreadAttr_t ext_led_blink_thread_attrs;
void ext_led_blink_thread_fn(void* arg);

osThreadId_t measurements_thread;
const osThreadAttr_t measurements_thread_attrs;
void measurements_thread_fn(void* arg);

osThreadId_t fsm_thread;
const osThreadAttr_t fsm_thread_attrs;
void fsm_thread_fn(void* arg);

osThreadId_t state_machine_thread;
const osThreadAttr_t state_machine_thread_attrs;
void StartStateMachine(void* arg);

#endif
