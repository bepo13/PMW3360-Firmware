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
#include <msp430.h>

#include "PMW3360.h"

#define POLLING_RATE        1000

#define CLOCK_FREQUENCY     8000000

PMW3360_data data;

static inline void __benchmarkStart(void)
{
    TA0CTL = TACLR;
    TA0CTL = TASSEL__SMCLK | MC__CONTINUOUS;
}

static inline uint16_t __benchmarkStop(void)
{
    uint16_t cycles = TA0R;
    TA0CTL = 0;
    return cycles;
}

static inline void EXP430FR5994_init(void)
{
    // Configure P1.0 for LED
    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // Disable the GPIO power-on default high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;

    // Clock System Setup, SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    // Per Device Errata set divider to 4 before changing frequency
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;
    CSCTL1 = DCOFSEL_6;                     // Set DCO to 8MHz

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);

    // Set all dividers to 1 for 8MHz operation
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;

    // Lock CS Registers
    CSCTL0_H = 0;
}

int main(void)
{
    uint16_t cycles;

    // Halt WDT
    WDTCTL = WDTPW | WDTHOLD;

    // Initialize MSP430FR5994 Launchpad
    EXP430FR5994_init();

    // Initialize PMW3360 sensor
    if (!PMW3360_init()) {
        // Error while initializing, blink LED
        while (1) {
            P1OUT ^= BIT0;
            __delay_cycles(CLOCK_FREQUENCY/10);
        }
    }

    while(1)
    {
        // Read data from PMW3360 sensor
        __benchmarkStart();
        PMW3360_read(&data);
        cycles = __benchmarkStop();

        // Turn on LED if sensor detects motion
        if (data.motion) {
            P1OUT |= BIT0;
        }
        else {
            P1OUT &= ~BIT0;
        }

        // Set a timer and go into LPM0
        TA0CCTL0 = CCIE;
        TA0CCR0 = (CLOCK_FREQUENCY/POLLING_RATE) - cycles;
        TA0CTL = TASSEL__SMCLK | MC__CONTINOUS;
        __bis_SR_register(LPM0_bits | GIE);
        __no_operation();
    }
}

// Timer0_A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    TA0CTL = 0;
    __bic_SR_register_on_exit(LPM0_bits | GIE);
}
