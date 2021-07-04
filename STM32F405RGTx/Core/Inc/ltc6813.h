#ifndef __LTC6813
#define __LTC6813

#include <stdint.h>
#include "main.h"

#ifndef LTC6813_BUFFER_SIZE
#define LTC6813_BUFFER_SIZE 	10 // bytes
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
uint8_t Buffer_check_pec(Buffer* self);

void Buffer_print(Buffer* self);
void Buffer_clear(Buffer* self);
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ltc6813 object
typedef struct {
	SPI_HandleTypeDef _spi_interface;
	GPIO_TypeDef* _cs_gpio_port;
	uint8_t _cs_pin_num;

	Buffer cmd_bfr;

	Buffer cfga_bfr;
	Buffer cfgb_bfr;

	Buffer cva_bfr;
	Buffer cvb_bfr;
	Buffer cvc_bfr;
	Buffer cvd_bfr;
	Buffer cve_bfr;;
	Buffer cvf_bfr;

	uint32_t timeout;
} Ltc6813;

// WARNING: Ltc6813_init will re-configure the CS pin to be used as basic GPIO,
//          meaning previous configurations will be broken...
Ltc6813 Ltc6813_init(SPI_HandleTypeDef spi, GPIO_TypeDef* cs_gpio_port, uint8_t cs_pin_num);

void Ltc6813_cs_low(Ltc6813* self);
void Ltc6813_cs_high(Ltc6813* self);

void Ltc6813_wakeup_sleep(Ltc6813* self);
void Ltc6813_wakeup_idle(Ltc6813* self);

void Ltc6813_print_voltages(Ltc6813* self);

void Ltc6813_write_spi(Ltc6813* self, Buffer* buffer);
void Ltc6813_read_spi(Ltc6813* self, Buffer* buffer);
//void Ltc6813_write_read_spi(Ltc6813* self, Buffer* buffer);

void Ltc6813_send_cmd(Ltc6813* self, uint16_t cmd);

uint8_t Ltc6813_read_reg(Ltc6813* self, uint8_t reg_cmd);
void Ltc6813_write_reg(Ltc6813* self, uint8_t reg_cmd);

uint8_t Ltc6813_read_cfga(Ltc6813* self);
uint8_t Ltc6813_read_cfgb(Ltc6813* self);
void Ltc6813_write_cfga(Ltc6813* self);
void Ltc6813_write_cfgb(Ltc6813* self);

uint8_t Ltc6813_read_adc(Ltc6813* self, uint16_t mode);

/*
cell_mask is a mask of 32 bits where the i-th bit represents whether or not a cell should be discharged
	- 1 -> balance cell, 0 -> don't balance cell.
	- only the first 18 LSBs are used, the remaining 14 are don't cares

ex. cell_mask = 0bXXXXXXXXXXXXXX000010000100001001
		==> cells 0, 3, 8, and 13 are being discharged, the rest are not
*/
uint8_t Ltc6813_discharge_ctrl(Ltc6813* self, uint32_t cell_mask);
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ltc6813 Command Code Defines
#ifndef __LTC6813_COMMAND_CODES

#define WRCFGA 		0b00000000001u
#define WRCFGB 		0b00000100100u

#define RDCFGA		0b00000000010u
#define RDCFGB		0b00000100110u

#define RDCVA	0b00000000100u
#define RDCVB	0b00000000110u
#define RDCVC	0b00000001000u
#define RDCVD	0b00000001010u
#define RDCVE	0b00000001001u
#define RDCVF	0b00000001011u

#define PLADC 	0b11100010100u

#endif


#ifndef __LTC6813_ADC_MODES

#define FAST_ADC 			0b01011100000u
#define FAST_ADC_DELAY		2

#define NORMAL_ADC 			0b01101100000u
#define NORMAL_ADC_DELAY	3

#define FILTERED_ADC 		0b01111100000u
#define FILTERED_ADC_DELAY	202

#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
