#ifndef __THREADS
#define __THREADS

#include "cmsis_os.h"

const osThreadAttr_t ext_led_blink_thread_attrs;
void ext_led_blink_thread_fn(void* arg);

const osThreadAttr_t measurements_thread_attrs;
void measurements_thread_fn(void* arg);

const osThreadAttr_t fsm_thread_attrs;
void fsm_thread_fn(void* arg);

#endif
