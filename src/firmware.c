
#include <string.h>
#include "system.h"
#include "ws2812b.h"

#define LED_PIN  7  // WiringPi pin number (GPIO 4 on RPi)

void gpio_setup(void) {
    setup_io();
    setup_gpio_output(LED_PIN);
}

int main(void) {
    system_setup();
    gpio_setup();

    RGB_t leds[8] = {0};
    WS2812B_t ws2812b = {
        .leds = leds,
        .num_leds = 8,
        .pin = LED_PIN
    };

    uint64_t start_time = system_get_ticks();
    uint8_t current_led = 0;

    while (1) {
        if (system_get_ticks() - start_time >= 250) {
            memset(&leds[current_led], 0, sizeof(RGB_t));
            current_led = (current_led + 1) % ws2812b.num_leds;
            leds[current_led].r = 255;

            WS2812B_Write(&ws2812b);
            start_time = system_get_ticks();
        }
    }

    return 0;
}

