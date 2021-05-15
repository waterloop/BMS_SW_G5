#ifndef __LTC6813
#define __LTC6813

#include "main.h"
#include "stdint.h"

#ifndef LTC6813_BUFFER_SIZE
#define LTC6813_BUFFER_SIZE 	100 // bytes
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer object
typedef struct {
	uint8_t len;
	uint8_t data[LTC6813_BUFFER_SIZE];
} Buffer;

Buffer Buffer_init();

void Buffer_append(Buffer* self, uint8_t val);

uint8_t Buffer_index(Buffer* self, uint8_t indx);
void Buffer_set_index(Buffer* self, uint8_t indx, uint8_t val);

void Buffer_add_pec(Buffer* self);	// calculates the PEC for the buffer and appends it to the end
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ltc6813 object
typedef struct {
	SPI_HandleTypeDef _spi_interface;
	GPIO_TypeDef* _cs_gpio_port;
	uint8_t _cs_pin_num;

	uint32_t timeout;
} Ltc6813;

// WARNING: Ltc6813_init will re-configure the CS pin to be used as basic GPIO,
//          meaning previous configurations will be broken...
Ltc6813 Ltc6813_init(SPI_HandleTypeDef spi, GPIO_TypeDef* cs_gpio_port, uint8_t cs_pin_num);

void Ltc6813_cs_low(Ltc6813* self);
void Ltc6813_cs_high(Ltc6813* self);

void Ltc6813_wakeup_sleep(Ltc6813* self);
void Ltc6813_wakeup_idle(Ltc6813* self);

void Ltc6813_read_cmd(Ltc6813* self, uint16_t command_code, Buffer* read_buffer);
void Ltc6813_write_cmd(Ltc6813* self, uint16_t command_code, Buffer* write_buffer);

void Ltc6813_await_adc_completion(Ltc6813* self);

void Ltc6813_write_spi(Ltc6813* self, Buffer* buffer);
void Ltc6813_read_spi(Ltc6813* self, Buffer* buffer);
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ltc6813 Command Code Defines

// #ifndef __LTC6813_COMMAND_CODES

// #define WRCFGA		0b00000000001u
// #define WRCFGB		0b00000100100u

// #define RDCFGA		0b00000000010u
// #define RDCFGB		0b00000100110u

// #define RDCVA		0b00000000100u
// #define RDCVB		0b00000000100u
// #define RDCVC		0b00000000110u
// #define RDCVC		0b00000001000u
// #define RDCVD		0b00000001010u
// #define RDCVE		0b00000001001u
// #define RDCVF		0b00000001011u

// #endif


/////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
