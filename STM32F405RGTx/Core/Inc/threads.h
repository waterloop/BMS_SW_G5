#ifndef __THREADS
#define __THREADS

#include "cmsis_os.h"

const osThreadAttr_t measurements_thread_attrs;
void measurements_thread_fn(void* arg);

#endif
