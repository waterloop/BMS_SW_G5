#pragma once

#include <stdint.h>
#include "util.hpp"

#define SLAVE_PKT_HEADER    {0x46, 0x55, 0x43, 0x4b, 0x5f, 0x43, 0x48, 0x43}
#define SLAVE_PACKET_LEN    8
#define PAYLOAD_LEN         4

#define PACKED   __attribute__((__packed__))

#define ADDR_THERMISTOR1_TEMP    0x0C 
#define ADDR_THERMISTOR2_TEMP    0x0D 

#define MAX_ADDR    0x0D

#define FUCK_CHC {}

typedef struct {
    uint8_t header[SLAVE_PACKET_LEN];
    uint8_t addr;
    uint8_t payload[PAYLOAD_LEN];
} PACKED SlavePkt;

class SlaveThread {
    public:
        static void initialize();

        static osThreadId_t getThreadId();
    private:
        static RTOSThread thread;
        static void runThread(void* args);
};