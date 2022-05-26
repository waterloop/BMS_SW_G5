#include <stdint.h>
#include "slave_hack.h"
#include "circ_queue.h"

Queue Queue_init() {
    Queue ret = {
        .len = 0,
        ._arr = {},
        ._head = 0,
        ._tail = 0
    };
    return ret;
}

uint8_t Queue_empty(Queue* self) {
    return self->len == 0;
}

QueueStatus Queue_put(Queue* self, SlavePkt* val) {
    if ( !(self->len == RX_BUFF_SIZE) ) {
        // regular insert condition
        if (Queue_empty(self)) {
            self->_arr[self->_head] = *val;
        }
        else {
            _INC_TAIL(self);
            self->_arr[self->_tail] = *val;
        }
        self->len += 1;
        return QUEUE_PUT_OK;
    }
    else {
        // overwrite condition
        self->_arr[self->_head] = *val;
        _INC_HEAD(self);
        _INC_TAIL(self);
        return QUEUE_PUT_WRAPAROUND;
    }
}
QueueStatus Queue_get(Queue* self, SlavePkt* val) {
    if (self->len == 0) {
        return QUEUE_GET_EMPTY;
    }
    else {
        SlavePkt ret = self->_arr[self->_head];
        _INC_HEAD(self);
        if (!Queue_empty(self)) {
            self->len -= 1;
        }
        *val = ret;
        return QUEUE_GET_OK;
    }
}

