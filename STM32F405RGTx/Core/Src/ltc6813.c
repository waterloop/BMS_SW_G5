#include <stdio.h>
#include <stdint.h>
#include "main.h"

#include "ltc6813.h"
#include "timer_utils.h"

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

const uint16_t _CRC15_LUT[256] = {
	0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,
	0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
	0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
	0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
	0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
	0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
	0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d,
	0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
	0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
	0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
	0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
	0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
	0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
	0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
	0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
	0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
	0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
	0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
	0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
	0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
	0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
	0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
	0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
};

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
		remainder = (remainder << 8)^_CRC15_LUT[addr];
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
		remainder = (remainder << 8)^_CRC15_LUT[addr];
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

void Ltc6813_print_voltages(Ltc6813* self) {

	printf("PRINTING CVA\r\n");
	Buffer_print(&(self->cva_bfr));

	printf("PRINTING CVB\r\n");
	Buffer_print(&(self->cvb_bfr));

	printf("PRINTING CVC\r\n");
	Buffer_print(&(self->cvc_bfr));

	printf("PRINTING CVD\r\n");
	Buffer_print(&(self->cvd_bfr));

	printf("PRINTING CVE\r\n");
	Buffer_print(&(self->cve_bfr));

	printf("PRINTING CVF\r\n");
	Buffer_print(&(self->cvf_bfr));

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

uint8_t Ltc6813_read_reg(Ltc6813* self, uint8_t reg_cmd) {

	Buffer* reg_buf;

	switch (reg_cmd) {
		case RDCFGA:
			reg_buf = &(self->cfga_bfr);
			break;
		case RDCFGB:
			reg_buf = &(self->cfgb_bfr);
			break;
		case RDCVA:
			reg_buf = &(self->cva_bfr);
			break;
		case RDCVB:
			reg_buf = &(self->cvb_bfr);
			break;
		case RDCVC:
			reg_buf = &(self->cvc_bfr);
			break;
		case RDCVD:
			reg_buf = &(self->cvd_bfr);
			break;
		case RDCVE:
			reg_buf = &(self->cve_bfr);
			break;
		case RDCVF:
			reg_buf = &(self->cvf_bfr);
			break;
		default:
			break;
	}

	Buffer_clear(reg_buf);

	reg_buf->len = 8;

	Ltc6813_cs_low(self);

	Ltc6813_send_cmd(self, reg_cmd);
	HAL_SPI_Receive(&self->_spi_interface, reg_buf->data, reg_buf->len, self->timeout);

	Ltc6813_cs_high(self);

	uint8_t pec_success = Buffer_check_pec(reg_buf);
	reg_buf->len = 6;

	return pec_success;

}

uint8_t Ltc6813_read_cfga(Ltc6813* self) {
	return Ltc6813_read_reg(self, RDCFGA);
}

uint8_t Ltc6813_read_cfgb(Ltc6813* self) {
	return Ltc6813_read_reg(self, RDCFGB);
}

void Ltc6813_write_cfga(Ltc6813* self) {

	Buffer_add_pec(&(self->cfga_bfr));

	Ltc6813_cs_low(self);

	Ltc6813_send_cmd(self, WRCFGA);
	HAL_SPI_Transmit(&self->_spi_interface, self->cfga_bfr.data, self->cfga_bfr.len, self->timeout);

	Ltc6813_cs_high(self);

	self->cfga_bfr.len = 6;
}

uint8_t Ltc6813_read_adc(Ltc6813* self, uint16_t mode) {

	Ltc6813_cs_low(self);

	Ltc6813_send_cmd(self, mode);

	uint32_t delay = FILTERED_ADC_DELAY;

	if (mode == FAST_ADC) {
		delay = FAST_ADC_DELAY;
	} else if (mode == NORMAL_ADC) {
		delay = NORMAL_ADC_DELAY;
	} else if (mode == FILTERED_ADC) {
		delay = FILTERED_ADC_DELAY;
	}

	osDelay(delay);

	uint8_t success = 1;

	success &= Ltc6813_read_reg(self, RDCVA);
	success &= Ltc6813_read_reg(self, RDCVB);
	success &= Ltc6813_read_reg(self, RDCVC);
	success &= Ltc6813_read_reg(self, RDCVD);
	success &= Ltc6813_read_reg(self, RDCVE);
	success &= Ltc6813_read_reg(self, RDCVF);

	return success;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
