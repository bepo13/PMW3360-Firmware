/* MIT License
 *
 * Copyright (c) 2023 Brent Peterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include <stdbool.h>

#include "PMW3360.h"
#include "PMW3360_port.h"
#include "PMW3360_firmware.h"

/*
 * Read register from PMW3360 sensor.
 */
static uint8_t PMW3360_readRegister(uint8_t address)
{
    uint8_t data;

    // Begin SPI transmission
    PMW3360_SPI_begin();

    // Write register address, delay 160us (tSRAD) and read register data
    PMW3360_SPI_readWrite(address & 0x7f);
    PMW3360_delayMicroseconds(160);
    data = PMW3360_SPI_readWrite(0);

    // End SPI transmission and delay 20us (tSRR)
    PMW3360_SPI_end();
    PMW3360_delayMicroseconds(20);

    return data;
}

/*
 * Write register to PMW3360 sensor.
 */
static void PMW3360_writeRegister(uint8_t address, uint8_t data)
{
    // Begin SPI transmission
    PMW3360_SPI_begin();

    // Write register address with MSB set indicating it's a write and send data
    PMW3360_SPI_readWrite(address | 0x80);
    PMW3360_SPI_readWrite(data);

    // End SPI transmission and delay 180us (tSWR)
    PMW3360_SPI_end();
    PMW3360_delayMicroseconds(180);

    return;
}

/*
 * Initialize the PMW3360 sensor.
 */
bool PMW3360_init(void)
{
    uint16_t i;
    uint16_t id;

    // Configure serial interface
    PMW3360_SPI_init();

    // Perform a hard reset and wait for sensor to reboot
    PMW3360_writeRegister(PMW3360_REG_POWER_UP_RESET, 0x5a);
    PMW3360_delayMicroseconds(50);

    // read registers 0x02-0x06
    PMW3360_readRegister(PMW3360_REG_MOTION);
    PMW3360_readRegister(PMW3360_REG_DELTA_X_L);
    PMW3360_readRegister(PMW3360_REG_DELTA_X_H);
    PMW3360_readRegister(PMW3360_REG_DELTA_Y_L);
    PMW3360_readRegister(PMW3360_REG_DELTA_Y_H);

    // Write 0 to Rest_En bit of Config2 register to disable rest mode
    PMW3360_writeRegister(PMW3360_REG_CONFIG2, 0x00);

    // Write 0x1d in SROM_enable register to initialize and delay 10ms
    PMW3360_writeRegister(PMW3360_REG_SROM_ENABLE, 0x1d);
    PMW3360_delayMicroseconds(10000);

    // Write 0x18 to SROM_enable register again to start SROM download and delay 120us
    PMW3360_writeRegister(PMW3360_REG_SROM_ENABLE, 0x18);
    PMW3360_delayMicroseconds(120);

    // Begin SPI transmission for load burst transfer
    PMW3360_SPI_begin();

    // Write to register to begin load burst transfer and delay 15us
    PMW3360_SPI_readWrite(PMW3360_REG_SROM_LOAD_BURST | 0x80);
    PMW3360_delayMicroseconds(15);

    // Write all bytes of the firmware image
    for (i = 0; i < sizeof(PMW3360_firmware); i++) {
        // Write data byte and delay 15us
        PMW3360_SPI_readWrite(PMW3360_firmware[i]);
        PMW3360_delayMicroseconds(15);
    }

    // End SPI transmission to signal end of burst load and delay 200us
    PMW3360_SPI_end();
    PMW3360_delayMicroseconds(200);

    // Read the SROM_ID register to verify the ID before any other register reads or writes
    id = PMW3360_readRegister(PMW3360_REG_SROM_ID);
    if (id == 0x04) {
        // Firmware load successful, write 0x00 to Config2 register for wired mouse design
        PMW3360_writeRegister(PMW3360_REG_CONFIG2, 0x00);

        // Set DPI to 800 by default
        PMW3360_writeRegister(PMW3360_REG_CONFIG1, 0x07);

        // Initialization successful
        return true;
    }
    else {
        // Initialization failed
        return false;
    }
}

/*
 * Shutdown the PMW3360 sensor.
 */
void PMW3360_shutdown(void)
{
    // Write 0xB6 to Shutdown register to start shutdown
    PMW3360_writeRegister(PMW3360_REG_SHUTDOWN, 0xb6);

    // Shutdown serial interface
    PMW3360_SPI_end();
    PMW3360_SPI_shutdown();

    return;
}

/*
 * Read one frame of motion data.
 */
void PMW3360_read(PMW3360_data *data)
{
    uint16_t i;
    uint8_t burstBuffer[12];

    // Begin burst transfer by writing to the Motion_Burst register
    PMW3360_writeRegister(PMW3360_REG_MOTION_BURST, 0);

    // Begin SPI transmission for burst mode and delay 35us (tSRAD_MOTBR)
    PMW3360_SPI_begin();
    PMW3360_SPI_readWrite(PMW3360_REG_MOTION_BURST);
    PMW3360_delayMicroseconds(35);

    // Read twelve bytes into the buffer with no delay
    for (i = 0; i < sizeof(burstBuffer); i++) {
        burstBuffer[i] = PMW3360_SPI_readWrite(0);
    }

    // Terminate burst transfer and delay 1us (tBEXIT)
    PMW3360_SPI_end();
    PMW3360_delayMicroseconds(1);

    // Calculate motion data
    data->motion = (burstBuffer[0] & 0x80) != 0;
    data->surface = (burstBuffer[0] & 0x08) == 0;
    data->dx = (int16_t)(((uint16_t)burstBuffer[3] << 8) + (uint16_t)burstBuffer[2]);
    data->dy = (int16_t)(((uint16_t)burstBuffer[5] << 8) + (uint16_t)burstBuffer[4]);
    data->SQUAL = burstBuffer[6];
    data->rawDataSum = burstBuffer[7];
    data->maxRawData = burstBuffer[8];
    data->minRawData = burstBuffer[9];
    data->shutter = ((uint16_t)burstBuffer[11] << 8) + (uint16_t)burstBuffer[10];

    return;
}

/*
 * Set the DPI level of the PMW3360 sensor.
 */
void PMW3360_setDPI(uint16_t dpi)
{
    int16_t val;

    // Calculate DPI value to write to register
    val = (dpi/100) - 1;

    // Check value is within range 0x00-0x77
    val = val < 0 ? 0 : (val < 0x77 ? val : 0x77);

    // Write DPI value to sensor register
    PMW3360_writeRegister(PMW3360_REG_CONFIG1, val);
}

/*
 * Get the DPI level of the PMW3360 sensor.
 */
uint16_t PMW3360_getDPI()
{
    uint16_t val;

    // Read config register to get DPI level
    val = PMW3360_readRegister(PMW3360_REG_CONFIG1);

    // Calculate DPI value and return result
    val = (val + 1)*100;
    return val;
}
