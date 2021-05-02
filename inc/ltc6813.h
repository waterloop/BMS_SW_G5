#include "main.h"
#include "stdint.h"

typedef struct {
	SPI_HandleTypeDef _spi_interface;
	uint32_t timeout;
} Ltc6813;

Ltc6813 Ltc6813_init();

void Ltc6813_write_spi(Ltc6813* self, uint8_t* cmd, uint8_t cmd_len);
void Ltc6813_read_spi(Ltc6813* self, uint8_t* output_buffer, uint8_t output_len);