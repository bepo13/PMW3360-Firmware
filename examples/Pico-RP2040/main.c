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

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "PMW3360.h"

PMW3360_data data;

#define PIN_LED     25

int main()
{
    // Initialize LED pin
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 0);

    // Initialize chosen serial port
    stdio_init_all();

    // Wait 10ms at startup for everything to settle
    sleep_ms(10);

    // Initialize PMW3360 sensor
    if (!PMW3360_init()) {
        // Error while initializing, blink LED
        while (1) {
            gpio_put(PIN_LED, 1);
            sleep_ms(1000);
            gpio_put(PIN_LED, 0);
            sleep_ms(1000);
        }
    }

    // main loop
    while (1) {
        // Read data from PMW3360 sensor
        PMW3360_read(&data);

        // Turn on LED if sensor detects motion
        if (data.motion) {
            gpio_put(PIN_LED, 1);
        }
        else {
            gpio_put(PIN_LED, 0);
        }

        // Wait 100ms
        sleep_ms(100);
    }
}