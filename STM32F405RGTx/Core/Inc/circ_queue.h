#pragma once

#include <stdint.h>
#include <stddef.h>
#include "slave_thread.hpp"

#define CIRC_RX_BUFF_SIZE    16

/*
Implementation of a circular CircQueue
*/

typedef enum {
    CircQueue_GET_EMPTY = 0,
    CircQueue_GET_OK = 1,
    CircQueue_PUT_OK = 2,
    CircQueue_PUT_WRAPAROUND = 3,
    CircQueue_ERROR = 0
} CircQueueStatus;

typedef struct {
    size_t len;

    SlavePkt _arr[CIRC_RX_BUFF_SIZE];
    size_t _head;
    size_t _tail;
} CircQueue;

#define CIRC_INC_HEAD(self) {                       \
    if (self->_head == (CIRC_RX_BUFF_SIZE - 1)) {    \
        self->_head = 0;                        \
    }                                           \
    else {                                      \
        self->_head += 1;                       \
    }                                           \
}
#define CIRC_INC_TAIL(self) {                       \
    if (self->_tail == (CIRC_RX_BUFF_SIZE - 1)) {    \
        self->_tail = 0;                        \
    }                                           \
    else {                                      \
        self->_tail += 1;                       \
    }                                           \
}

CircQueue CircQueue_init();
uint8_t CircQueue_empty(CircQueue* self);
CircQueueStatus CircQueue_put(CircQueue* self, SlavePkt* val);
CircQueueStatus CircQueue_get(CircQueue* self, SlavePkt* val);

void CircQueue_print(CircQueue* self);

