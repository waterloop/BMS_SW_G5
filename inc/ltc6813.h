#include "main.h"
#include "stdint.h"

typedef struct {
	SPI_HandleTypeDef spi;
	uint32_t timeout;
} SlaveDevice;

SlaveDevice SlaveDevice_init(uint32_t timeout);


