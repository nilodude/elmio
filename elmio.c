#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

const uint LED_PIN = 25;
const uint BUTTON1_PIN = 26;
const uint BUTTON2_PIN = 24;
const uint LEDS_PIN = 27;

int main() {

    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    bi_decl(bi_1pin_with_name(BUTTON1_PIN, "button 1"));
    bi_decl(bi_1pin_with_name(BUTTON2_PIN, "button 2"));
    bi_decl(bi_1pin_with_name(LEDS_PIN, "neopixel strip"));

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(LEDS_PIN);
    gpio_set_dir(LEDS_PIN, GPIO_OUT);

    gpio_init(BUTTON1_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);

    gpio_init(BUTTON2_PIN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);

    while (1) {
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        gpio_put(LED_PIN, 1);
        puts("vamono que nos vamo\n");

        sleep_ms(1000);
    }
}