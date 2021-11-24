#include <stdio.h>
#include <stdint.h>
#include "main.h"

#include "lut.h"
#include "ltc6813.h"
#include "timer_utils.h"
#include "cmsis_os.h"

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
    if (indx >= self->len) {
        printf("Error: tried to get a buffer index that's out of range...\r\n");
        Error_Handler();
    }
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

    slave_device.cva_bfr = Buffer_init();
    slave_device.cvb_bfr = Buffer_init();
    slave_device.cvc_bfr = Buffer_init();
    slave_device.cvd_bfr = Buffer_init();
    slave_device.cve_bfr = Buffer_init();
    slave_device.cvf_bfr = Buffer_init();

    slave_device.timeout = 10000;

    Ltc6813_cs_high(&slave_device);

    osMutexAttr_t mutex_attrs = {
        "spi_mutex",
        osMutexRecursive | osMutexPrioInherit,
        NULL,    // memory for control block   
        0U        // size for control block
    };
    slave_device._spi_mutex = osMutexNew(&mutex_attrs);
    if (slave_device._spi_mutex == NULL) {
        printf("Error: failed to create SPI mutex for LTC6813...\r\n");
        Error_Handler();
    }

    return slave_device;
}

void _Ltc6813_acquire_mutex(Ltc6813* self) {
    if (osMutexAcquire(self->_spi_mutex, osWaitForever) != osOK) {
        printf(
            "Error: failed to aquire '_spi_mutex' from Ltc6813 object at %p...\r\n",
            (void*)self );
        Error_Handler();
    }
}
void _Ltc6813_release_mutex(Ltc6813* self) {
    if (osMutexRelease(self->_spi_mutex) != osOK) {
        printf(
            "Error: failed to release '_spi_mutex' from Ltc6813 object at %p...\r\n",
            (void*)self );
        Error_Handler();
    }
}
void _Ltc6813_decode_adc(Ltc6813* self) {
	/*
	I'm not using Buffer_index here since I want
	this subroutine to be as fast as possible.
	*/

    // CVA
    uint16_t c1v = (self->cva_bfr.data[1] << 8) | self->cva_bfr.data[0];
    uint16_t c2v = (self->cva_bfr.data[3] << 8) | self->cva_bfr.data[2];
    uint16_t c3v = (self->cva_bfr.data[5] << 8) | self->cva_bfr.data[4];

    // CVB
    uint16_t c4v = (self->cvb_bfr.data[1] << 8) | self->cvb_bfr.data[0];
    uint16_t c5v = (self->cvb_bfr.data[3] << 8) | self->cvb_bfr.data[2];
    uint16_t c6v = (self->cvb_bfr.data[5] << 8) | self->cvb_bfr.data[4];

    // CVC
    uint16_t c7v = (self->cvc_bfr.data[1] << 8) | self->cvc_bfr.data[0];
    uint16_t c8v = (self->cvc_bfr.data[3] << 8) | self->cvc_bfr.data[2];
    uint16_t c9v = (self->cvc_bfr.data[5] << 8) | self->cvc_bfr.data[4];

    // CVD
    uint16_t c10v = (self->cvd_bfr.data[1] << 8) | self->cvd_bfr.data[0];
    uint16_t c11v = (self->cvd_bfr.data[3] << 8) | self->cvd_bfr.data[2];
    uint16_t c12v = (self->cvd_bfr.data[5] << 8) | self->cvd_bfr.data[4];

    // CVE
    uint16_t c13v = (self->cve_bfr.data[1] << 8) | self->cve_bfr.data[0];
    uint16_t c14v = (self->cve_bfr.data[3] << 8) | self->cve_bfr.data[2];
    uint16_t c15v = (self->cve_bfr.data[5] << 8) | self->cve_bfr.data[4];

    // CVF
    uint16_t c16v = (self->cvf_bfr.data[1] << 8) | self->cvf_bfr.data[0];
    uint16_t c17v = (self->cvf_bfr.data[3] << 8) | self->cvf_bfr.data[2];
    uint16_t c18v = (self->cvf_bfr.data[5] << 8) | self->cvf_bfr.data[4];

    self->cell_voltages[0] = c1v*100E-6;
    self->cell_voltages[1] = c2v*100E-6;
    self->cell_voltages[2] = c3v*100E-6;
    self->cell_voltages[3] = c4v*100E-6;
    self->cell_voltages[4] = c5v*100E-6;
    self->cell_voltages[5] = c6v*100E-6;
    self->cell_voltages[6] = c7v*100E-6;
    self->cell_voltages[7] = c8v*100E-6;
    self->cell_voltages[8] = c9v*100E-6;
    self->cell_voltages[9] = c10v*100E-6;
    self->cell_voltages[10] = c11v*100E-6;
    self->cell_voltages[11] = c12v*100E-6;
    self->cell_voltages[12] = c13v*100E-6;
    self->cell_voltages[13] = c14v*100E-6;
    self->cell_voltages[14] = c15v*100E-6;
    self->cell_voltages[15] = c16v*100E-6;
    self->cell_voltages[16] = c17v*100E-6;
    self->cell_voltages[17] = c18v*100E-6;
}

void Ltc6813_cs_low(Ltc6813* self) { HAL_GPIO_WritePin(self->_cs_gpio_port, (1u << self->_cs_pin_num), 0); }
void Ltc6813_cs_high(Ltc6813* self) { HAL_GPIO_WritePin(self->_cs_gpio_port, (1u << self->_cs_pin_num), 1); }

// WAKEUP FUNCTIONS:
// setting CS low will send a long isoSPI pulse (reference: page 18 of LTC6820 datasheet)
void Ltc6813_wakeup_sleep(Ltc6813* self) {
    Ltc6813_cs_low(self);
    delay_us(410);        // according to datasheet, t_wake = 400us
    Ltc6813_cs_high(self);
    delay_us(30);
}
void Ltc6813_wakeup_idle(Ltc6813* self) {
    Ltc6813_cs_low(self);
    delay_us(20);        // according to datasheet, t_wake = 10us

    uint8_t buff[1] = {0xff};

    _Ltc6813_acquire_mutex(self);
    HAL_SPI_Receive(&self->_spi_interface, buff, 1, self->timeout);
    _Ltc6813_release_mutex(self);

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

    _Ltc6813_acquire_mutex(self);
    HAL_SPI_Transmit(&self->_spi_interface, self->cmd_bfr.data, self->cmd_bfr.len, self->timeout);
    _Ltc6813_release_mutex(self);
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
            reg_buf = NULL;
            break;
    }

    Buffer_clear(reg_buf);
    reg_buf->len = 8;

    Ltc6813_cs_low(self);

    Ltc6813_send_cmd(self, reg_cmd);

    _Ltc6813_acquire_mutex(self);
    HAL_SPI_Receive(&self->_spi_interface, reg_buf->data, reg_buf->len, self->timeout);
    _Ltc6813_release_mutex(self);

    Ltc6813_cs_high(self);

    uint8_t pec_success = Buffer_check_pec(reg_buf);
    reg_buf->len = 6;

    return pec_success;

}
void Ltc6813_write_reg(Ltc6813* self, uint8_t reg_cmd) {
    Buffer* reg_buff;
    if (reg_cmd == WRCFGA) {
        reg_buff = &self->cfga_bfr;
    }
    else if (reg_cmd == WRCFGB) {
        reg_buff = &self->cfgb_bfr;
    }
    else {
        reg_buff = NULL;
        printf("Error: command code %d is not implemented yet...", reg_cmd);
        Error_Handler();
    }
    Buffer_add_pec(reg_buff);
    Ltc6813_cs_low(self);
    Ltc6813_send_cmd(self, reg_cmd);

    _Ltc6813_acquire_mutex(self);
    HAL_SPI_Transmit(&self->_spi_interface, reg_buff->data, reg_buff->len, self->timeout);
    _Ltc6813_release_mutex(self);

    Ltc6813_cs_high(self);
    reg_buff->len = 6;
}

uint8_t Ltc6813_read_cfga(Ltc6813* self) { return Ltc6813_read_reg(self, RDCFGA); }
uint8_t Ltc6813_read_cfgb(Ltc6813* self) { return Ltc6813_read_reg(self, RDCFGB); }
void Ltc6813_write_cfga(Ltc6813* self) { return Ltc6813_write_reg(self, WRCFGA); }
void Ltc6813_write_cfgb(Ltc6813* self) { return Ltc6813_write_reg(self, WRCFGB); }

uint8_t Ltc6813_read_adc(Ltc6813* self, uint16_t mode) {
    Ltc6813_cs_low(self);
    Ltc6813_send_cmd(self, mode);
    Ltc6813_cs_high(self);

    // Wait for references to power up. Should be 4.4 ms, but can only delay integer ticks (1ms/tick)
    osDelay(5);


    uint32_t delay = FILTERED_ADC_DELAY;
    switch (mode) {
        case (FAST_ADC):
            delay = FAST_ADC_DELAY;
            break;
        case (NORMAL_ADC):
            delay = NORMAL_ADC_DELAY;
            break;
        case (FILTERED_ADC):
            delay = FILTERED_ADC_DELAY;
            break;
    }
    osDelay(delay);

    uint8_t success = 1;

    Ltc6813_wakeup_idle(self);

    success &= Ltc6813_read_reg(self, RDCVA);
    success &= Ltc6813_read_reg(self, RDCVB);
    success &= Ltc6813_read_reg(self, RDCVC);
    success &= Ltc6813_read_reg(self, RDCVD);
    success &= Ltc6813_read_reg(self, RDCVE);
    success &= Ltc6813_read_reg(self, RDCVF);

    _Ltc6813_decode_adc(self);

    return success;
}

uint8_t Ltc6813_discharge_ctrl(Ltc6813* self, uint32_t cell_mask) {
    // CFGAR4 contains DCC[8:1]
    uint8_t cfgar4 = cell_mask & 0b11111111U;
    Buffer_set_index(&self->cfga_bfr, 4U, cfgar4);

    // CFGAR5 contains DCC[12:9] in the 4 LSBs
    uint8_t cfgar5 = Buffer_index(&self->cfga_bfr, 5U);
    cfgar5 &= ~(0b1111U);
    cfgar5 |= (cell_mask >> 8U) & 0b1111U;
    Buffer_set_index(&self->cfga_bfr, 5U, cfgar5);

    // CFGBR0 contains DCC[16:13] in the 4 MSBs
    uint8_t cfgbr0 = Buffer_index(&self->cfgb_bfr, 0U);
    cfgbr0 &= ~(0b1111U << 4U);
    cfgbr0 |= (cell_mask >> 12U) & 0b1111U;
    Buffer_set_index(&self->cfgb_bfr, 0U, cfgbr0);

    // CFGBR1 contains DCC[18:17] in the 2 LSBs
    uint8_t cfgbr1 = Buffer_index(&self->cfgb_bfr, 1U);
    cfgbr1 &= ~(0b11U);
    cfgbr1 |= (cell_mask >> 16U) & 0b11U;
    Buffer_set_index(&self->cfgb_bfr, 1U, cfgbr1);

    // write back to the device
    uint8_t status = 1U;

    /*
    TODO:
        - eventually, all of the write functions should have return codes,
          maybe return a HAL_StatusTypeDef
    */

    Ltc6813_write_cfga(self);
    Ltc6813_write_cfga(self);
    Ltc6813_write_cfgb(self);

    return status;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
