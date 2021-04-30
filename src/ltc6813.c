#include "main.h"
#include "ltc6813.h"

SlaveDevice SlaveDevice_init(uint32_t timeout) {
	extern SPI_HandleTypeDef hspi2;

	SlaveDevice slave_device = {};
	slave_device.spi = hspi2;
	slave_device.timeout = timeout;

	return slave_device;
}

