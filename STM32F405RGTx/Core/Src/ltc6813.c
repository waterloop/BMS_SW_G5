#include <stdio.h>
#include <stdint.h>
#include "main.h"

#include "ltc6813.h"
#include "timer_utils.h"
#include "lut.h"

/*
IMPORTANT TIMING PARAMETERS:
	- system core:
		- t_sleep = 1.8s (min) to 2.2s (max)
			- time for the chip's watchdog to kick in and transition the core to SLEEP state
		- t_wake = 400us (max)
			- time for the chip's core to transition from the SLEEP state to the STANDBY state
		- t_refup = 2.7ms (min) to 4.4ms (max)
			- time for the chip's core to transition DIRECTLY from the STANDBY state to the MEASURE state
	isoSPI Phy:
		- t_ready = 10us (max)
			- time for the chip's isoSPI tranceiver to transition to its READY state after differential activity is detected
		- t_idle = 4.3ms (min) to 6.7ms (max)
			- time for the chip's isoSPI tranceiver to transition into its IDLE state
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer methods
Buffer Buffer_init() {
	Buffer buffer = {};
	buffer.len = 0;
	return buffer;
}

void Buffer_append(Buffer* self, uint8_t val) {
	uint8_t indx = self->len;
	self->data[indx] = val;
	self->len += 1;
}

uint8_t Buffer_index(Buffer* self, uint8_t indx) {
	if (indx >= self->len) { Error_Handler(); }
	return self->data[indx];
}
void Buffer_set_index(Buffer* self, uint8_t indx, uint8_t val) {
	if (indx >= self->len) { Error_Handler(); }
	self->data[indx] = val;
}

void Buffer_add_pec(Buffer* self) {
	uint16_t remainder = 16;
	uint16_t addr = 0;
	for (uint8_t i = 0; i < self->len; i++) {
		addr = ( (remainder >> 7)^Buffer_index(self, i) ) & 0xff;
		remainder = (remainder << 8)^CRC15_LUT[addr];
	}
	uint16_t pec = remainder*2;
	Buffer_append(self, (pec >> 8) & 0xff);
	Buffer_append(self, pec & 0xff);
}

uint8_t Buffer_check_pec(Buffer* self) {
	uint16_t remainder = 16;
	uint16_t addr = 0;
	for (uint8_t i = 0; i < self->len - 2; i++) {
		addr = ( (remainder >> 7)^Buffer_index(self, i) ) & 0xff;
		remainder = (remainder << 8)^CRC15_LUT[addr];
	}
	uint16_t calc_pec = remainder*2;
	uint16_t act_pec = ((uint16_t)Buffer_index(self, self->len - 2) << 8) | Buffer_index(self, self->len - 1);
	return calc_pec == act_pec;
}

void Buffer_print(Buffer* self) {
	char str[500];
	for (uint8_t i = 0; i < self->len; i++) {
		sprintf(str, "pkt byte %d: %d\r\n", i, Buffer_index(self, i));
		printf(str);
	}
	printf("\r\n");
}

void Buffer_clear(Buffer* self) {
	while (self->len > 0) {
		Buffer_set_index(self, self->len - 1, 0);
		self->len = self->len - 1;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ltc6813 methods
Ltc6813 Ltc6813_init(SPI_HandleTypeDef spi, GPIO_TypeDef* cs_gpio_port, uint8_t cs_pin_num) {
	Ltc6813 slave_device = {};
	slave_device._spi_interface = spi;

	// config CS pin as GPIO output
	cs_gpio_port->MODER &= ~(0b11u << (cs_pin_num*2));
	cs_gpio_port->MODER |= (0b01u << (cs_pin_num*2));

	slave_device._cs_gpio_port = cs_gpio_port;
	slave_device._cs_pin_num = cs_pin_num;

	slave_device.cmd_bfr = Buffer_init();

	slave_device.cfga_bfr = Buffer_init();
	slave_device.cfgb_bfr = Buffer_init();

	slave_device.timeout = 10000;

	Ltc6813_cs_high(&slave_device);

	return slave_device;
}

void Ltc6813_cs_low(Ltc6813* self) { HAL_GPIO_WritePin(self->_cs_gpio_port, (1u << self->_cs_pin_num), 0); }
void Ltc6813_cs_high(Ltc6813* self) { HAL_GPIO_WritePin(self->_cs_gpio_port, (1u << self->_cs_pin_num), 1); }

// WAKEUP FUNCTIONS:
// setting CS low will send a long isoSPI pulse (reference: page 18 of LTC6820 datasheet)
void Ltc6813_wakeup_sleep(Ltc6813* self) {
	Ltc6813_cs_low(self);
	delay_us(410);		// according to datasheet, t_wake = 400us
	Ltc6813_cs_high(self);
	delay_us(30);
}
void Ltc6813_wakeup_idle(Ltc6813* self) {
	Ltc6813_cs_low(self);
	delay_us(20);		// according to datasheet, t_wake = 10us
	Ltc6813_cs_high(self);
}

// READ COMMAND FUNCTIONS:
// commands to send read commands and receive data back (page 60 of LTC6813 datasheet)
void Ltc6813_send_cmd(Ltc6813* self, uint16_t cmd) {
	Buffer_clear(&self->cmd_bfr);

	Buffer_append(&self->cmd_bfr, (cmd >> 8) & 0xff);
	Buffer_append(&self->cmd_bfr, cmd & 0xff);

	Buffer_add_pec(&self->cmd_bfr);

	HAL_SPI_Transmit(&self->_spi_interface, self->cmd_bfr.data, self->cmd_bfr.len, self->timeout);
}

uint8_t Ltc6813_read_cfga(Ltc6813* self) {
	Buffer_clear(&self->cfga_bfr);

	self->cfga_bfr.len = 8;

	Ltc6813_cs_low(self);

	Ltc6813_send_cmd(self, RDCFGA);
	HAL_SPI_Receive(&self->_spi_interface, self->cfga_bfr.data, self->cfga_bfr.len, self->timeout);

	Ltc6813_cs_high(self);

	uint8_t pec_success = Buffer_check_pec(&self->cfga_bfr);
	self->cfga_bfr.len = 6;

	return pec_success;
}

uint8_t Ltc6813_read_cfgb(Ltc6813* self) {
	Buffer_clear(&self->cfgb_bfr);

	self->cfgb_bfr.len = 8;

	Ltc6813_cs_low(self);

	Ltc6813_send_cmd(self, RDCFGB);
	HAL_SPI_Receive(&self->_spi_interface, self->cfgb_bfr.data, self->cfgb_bfr.len, self->timeout);

	Ltc6813_cs_high(self);

	uint8_t pec_success = Buffer_check_pec(&self->cfgb_bfr);
	self->cfgb_bfr.len = 6;

	return pec_success;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
