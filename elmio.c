#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

const uint LED_PIN = 25; //GPIO25
const uint BUTTON1_PIN = 16; //GPIO16
const uint BUTTON2_PIN = 17; //GPIO17
const uint LEDS_PIN = 15; //GPIO15
const uint NUMPIXELS = 2;

const uint pins[11]={16,17,0,0,0,0,0,0,0,0,0};

struct Button {
  uint pin;
  uint32_t color;
  bool pressed;
};

struct Button buttons[11] = {};

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) |
         ((uint32_t)(g) << 16) |
         (uint32_t)(b);
}

static inline void setup(){
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    bi_decl(bi_1pin_with_name(BUTTON1_PIN, "button 1"));
    bi_decl(bi_1pin_with_name(BUTTON2_PIN, "button 2"));
    bi_decl(bi_1pin_with_name(LEDS_PIN, "neopixel strip"));

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    //should iterate when > 1 "led strip"
    gpio_init(LEDS_PIN);
    gpio_set_dir(LEDS_PIN, GPIO_OUT);

    gpio_init(BUTTON1_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON1_PIN);

    gpio_init(BUTTON2_PIN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON2_PIN);

    for (int i = 0; i < NUMPIXELS; i++) {
      struct Button button = {pins[i],urgb_u32(0x05, 0x10, 0x05),false};
      buttons[i] = button;
    }
}

int main() {

    setup();

    //each strip can address 8 pixels, maybe for more than 
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    char str[12];

    ws2812_program_init(pio, sm, offset, LEDS_PIN, 800000, false);

    bool shouldSwitch = false;
    bool button1pressed = false;
    bool button2pressed = false;

    while (1) {
        gpio_put(LED_PIN, !gpio_get(BUTTON1_PIN) || !gpio_get(BUTTON2_PIN));
        
        //BUTTON1
        while(!gpio_get(buttons[0].pin)){
          buttons[0].pressed = true;
        }
        if(buttons[0].pressed){
          shouldSwitch = !shouldSwitch;
          buttons[0].pressed = false;
        }

        //BUTTON2
        while(!gpio_get(buttons[1].pin)){
          buttons[1].pressed = true;
        }
        if(buttons[1].pressed){
          shouldSwitch = !shouldSwitch;
          buttons[1].pressed = false;
        }        
        
        // for (int i = 0; i < NUMPIXELS; i++) {
        //   struct Button b = buttons[i];
          
        //   while(!gpio_get(b.pin)){
        //     b.pressed = true; 
        //   }
        // }

        // for (int i = 0; i < NUMPIXELS; i++) {
        //   struct Button b = buttons[i];

        //   if(b.pressed){
        //     shouldSwitch = !shouldSwitch;
        //     b.pressed = false;
        //     break;
        //   }
        // }


        for (int i = 0; i < NUMPIXELS; i++) {
          put_pixel(shouldSwitch ? urgb_u32(0x05, 0x10, 0x05) : urgb_u32(0x05, 0, 0x10));
          put_pixel(!shouldSwitch ? urgb_u32(0x05, 0x10, 0x05) : urgb_u32(0x05, 0, 0x10));
        }

        sleep_ms(1);
        
        printf("%s\n", shouldSwitch ? "lima" : "moraito");
        
    }
}