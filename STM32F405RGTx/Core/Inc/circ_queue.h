#pragma once

#include <stdint.h>
#include <stddef.h>
#include "slave_hack.h"

#define RX_BUFF_SIZE    16

/*
Implementation of a circular queue
*/

typedef enum {
    QUEUE_GET_EMPTY = 0,
    QUEUE_GET_OK = 1,
    QUEUE_PUT_OK = 2,
    QUEUE_PUT_WRAPAROUND = 3,
    QUEUE_ERROR = 0
} QueueStatus;

typedef struct {
    size_t len;

    SlavePkt _arr[RX_BUFF_SIZE];
    size_t _head;
    size_t _tail;
} Queue;

#define _INC_HEAD(self) {                       \
    if (self->_head == (RX_BUFF_SIZE - 1)) {    \
        self->_head = 0;                        \
    }                                           \
    else {                                      \
        self->_head += 1;                       \
    }                                           \
}
#define _INC_TAIL(self) {                       \
    if (self->_tail == (RX_BUFF_SIZE - 1)) {    \
        self->_tail = 0;                        \
    }                                           \
    else {                                      \
        self->_tail += 1;                       \
    }                                           \
}

Queue Queue_init();
uint8_t Queue_empty(Queue* self);
QueueStatus Queue_put(Queue* self, SlavePkt* val);
QueueStatus Queue_get(Queue* self, SlavePkt* val);

void Queue_print(Queue* self);

