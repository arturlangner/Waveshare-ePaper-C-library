#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
* This function configures the SPI bus and pins for: reset, chip select, data/command, busy signals.
* It pulls reset low for 200ms, pulls high and waits 200ms. This function is blocking.
*/
void disp_hw_init(void);

/**
* This function disables the peripherals connected to the display,
* disconnects power to the display etc. It is optional.
*/
void disp_hw_deinit(void);

/**
* Sends data over the SPI bus to the display. This function is blocking.
* @param data Pointer to data to be written.
* @param length Length of data to be written (in bytes)
*/
void disp_hw_spi_write_blocking(const uint8_t *data, uint32_t length);

/**
 * Pulls the chip select signal low.
 */
void disp_hw_cs_low(void);

/**
 * Pulls the chip select signal high.
 */
void disp_hw_cs_high(void);

/**
 * Polls the BUSY signal from the display. This function is blocking.
 * It can either user polling or configure the GPIO for interrupt sensing.
 * This function should also enter sleep mode while waiting for reduced power consumption.
 */
void disp_hw_busy_wait(void);

/**
* Sets the DATA/COMMAND line
* @param write_is_command true if next write should be treated as data, false if it should be treated as command
*/
void disp_hw_set_write_mode(bool write_is_data);
