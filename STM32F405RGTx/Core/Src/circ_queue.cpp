#include <stdint.h>
#include "circ_queue.h"

CircQueue CircQueue_init() {
    CircQueue ret = {
        .len = 0,
        ._arr = {},
        ._head = 0,
        ._tail = 0
    };
    return ret;
}

uint8_t CircQueue_empty(CircQueue* self) {
    return self->len == 0;
}

CircQueueStatus CircQueue_put(CircQueue* self, SlavePkt* val) {
    if ( !(self->len == CIRC_RX_BUFF_SIZE) ) {
        // regular insert condition
        if (CircQueue_empty(self)) {
            self->_arr[self->_head] = *val;
        }
        else {
            CIRC_INC_TAIL(self);
            self->_arr[self->_tail] = *val;
        }
        self->len += 1;
        return CircQueue_PUT_OK;
    }
    else {
        // overwrite condition
        self->_arr[self->_head] = *val;
        CIRC_INC_HEAD(self);
        CIRC_INC_TAIL(self);
        return CircQueue_PUT_WRAPAROUND;
    }
}
CircQueueStatus CircQueue_get(CircQueue* self, SlavePkt* val) {
    if (self->len == 0) {
        return CircQueue_GET_EMPTY;
    }
    else {
        SlavePkt ret = self->_arr[self->_head];
        CIRC_INC_HEAD(self);
        if (!CircQueue_empty(self)) {
            self->len -= 1;
        }
        *val = ret;
        return CircQueue_GET_OK;
    }
}

