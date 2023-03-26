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

#ifdef __MSP430FR5994__

#include <msp430.h>

#define PMW3360_delayMicroseconds(x)    (__delay_cycles(x<<3))

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

#endif //__MSP430FR5994__

#endif //PMW3360_PORT_H__
