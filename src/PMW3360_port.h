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

#ifndef PMW3360_PORT_H__
#define PMW3360_PORT_H__

#include <stdint.h>
#include <stdbool.h>

#if defined(__PICO_SDK__)

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/resets.h"

#define PMW3360_delayMicroseconds(x)    (sleep_us(x))

#define PIN_SCK     2
#define PIN_MOSI    3
#define PIN_MISO    4
#define PIN_CS      5

#define SPI_PORT    spi0

static inline void PMW3360_SPI_init()
{
    // Configure chip select pin
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Configure SPI for 1MHz
    spi_init(SPI_PORT, 1000000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
}

static inline void PMW3360_SPI_shutdown()
{
    // Deinitialize SPI
    spi_deinit(SPI_PORT);
}

static inline void PMW3360_SPI_begin(void)
{
    // Set chip select pin low
    gpio_put(PIN_CS, 0);
    PMW3360_delayMicroseconds(1);
}

static inline void PMW3360_SPI_end(void)
{
    // Set chip select pin high
    PMW3360_delayMicroseconds(1);
    gpio_put(PIN_CS, 1);
}

static inline uint8_t PMW3360_SPI_readWrite(uint8_t data)
{
    uint8_t result;

    // Simultaniously write and read a byte
    spi_write_read_blocking(SPI_PORT, &data, &result, 1);

    // Return receives data
    return result;
}

#elif defined(__MSP430FR5994__)

#include <msp430.h>

#define PMW3360_delayMicroseconds(x)    (__delay_cycles(x<<3))  // MCLK @ 8MHz

static inline void PMW3360_SPI_init()
{
    // Configure chip select pin
    P5OUT &= ~BIT3;
    P5DIR |= BIT3;

    // Configure CLK, MOSI and MISO pins
    P5SEL1 &= ~(BIT0 | BIT1 | BIT2);
    P5SEL0 |= (BIT0 | BIT1 | BIT2);

    // Configure USCI_B1 for SPI operation (3-pin, 8-bit, MSB)
    UCB1CTLW0 = UCSWRST;
    UCB1CTLW0 |= UCMST | UCSYNC | UCCKPL | UCMSB;
    UCB1CTLW0 |= UCSSEL__SMCLK;
    UCB1BRW = 0x08;
    UCB1CTLW0 &= ~UCSWRST;
}

static inline void PMW3360_SPI_shutdown()
{
    // Put USCI_B1 in software reset
    UCB1CTLW0 = UCSWRST;
}

static inline void PMW3360_SPI_begin(void)
{
    // Set chip select pin low
    P5OUT &= ~BIT3;
    PMW3360_delayMicroseconds(1);
}

static inline void PMW3360_SPI_end(void)
{
    // Set chip select pin high
    PMW3360_delayMicroseconds(1);
    P5OUT |= BIT3;
}

static inline uint8_t PMW3360_SPI_readWrite(uint8_t data)
{
    // Wait for transmit buffer to clear and transmit data byte
    while(!(UCB1IFG & UCTXIFG));
    UCB1TXBUF = data;

    // Wait for receive buffer to fill and receive data byte
    while (!(UCB1IFG & UCRXIFG));
    data = UCB1RXBUF;

    // Return received data
    return data;
}

#endif

#endif //PMW3360_PORT_H__
