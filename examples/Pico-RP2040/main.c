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