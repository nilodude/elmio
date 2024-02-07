#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
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

const uint ENCODER_CLK = 2;
const uint ENCODER_DT = 3;
const uint ENCODER_SW = 4;

bool currCLK = 0;
uint counter = 0;
bool lastCLK = 0;
unsigned long lastButtonPress = 0;
bool pushButton = 0;

bool shouldSwitch = false; //testing led switching when buttons clicked

const uint SEG_1 = 10;
const uint SEG_2 = 11;
const uint SEG_3 = 12;
const uint SEG_4 = 13;
const uint SEG_A = 15;
const uint SEG_B = 14;
const uint SEG_C = 9;
const uint SEG_D = 8;
const uint SEG_E = 7;
const uint SEG_F = 6;
const uint SEG_G = 5;
const uint SEG_H = 1;

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

    //LED BUTTONS
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

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

    //ENCODER
    gpio_init(ENCODER_CLK);
    gpio_set_dir(ENCODER_CLK, GPIO_IN);
    gpio_pull_up(ENCODER_CLK);
    lastCLK = gpio_get(ENCODER_CLK);

    gpio_init(ENCODER_DT);
    gpio_set_dir(ENCODER_DT, GPIO_IN);
    gpio_pull_up(ENCODER_DT);

    gpio_init(ENCODER_SW);
    gpio_set_dir(ENCODER_SW, GPIO_IN);
    gpio_pull_up(ENCODER_SW);

    //7 SEGMENT DISPLAY
    gpio_init(SEG_1);
    gpio_set_dir(SEG_1, GPIO_OUT);

    gpio_init(SEG_2);
    gpio_set_dir(SEG_2, GPIO_OUT);

    gpio_init(SEG_3);
    gpio_set_dir(SEG_3, GPIO_OUT);

    gpio_init(SEG_4);
    gpio_set_dir(SEG_4, GPIO_OUT);

    gpio_init(SEG_A);
    gpio_set_dir(SEG_A, GPIO_OUT);

    gpio_init(SEG_B);
    gpio_set_dir(SEG_B, GPIO_OUT);

    gpio_init(SEG_C);
    gpio_set_dir(SEG_C, GPIO_OUT);
}

static inline void handleEncoder(){
  currCLK = gpio_get(ENCODER_CLK);
        
  if (currCLK != lastCLK && currCLK){
    if(gpio_get(ENCODER_DT) != currCLK){
      counter--;
    }else{
      counter++;
    }
    printf("%d\n",counter);
  }
  
  lastCLK = gpio_get(ENCODER_CLK);
  
  pushButton = gpio_get(ENCODER_SW);

  if(!gpio_get(ENCODER_SW)){
    if(to_ms_since_boot(get_absolute_time()) - lastButtonPress > 50){
      printf("pulsao\n");
    }
    lastButtonPress = to_ms_since_boot(get_absolute_time());
  }
}

static inline void handleButtons(){
  //BUTTON1
  while(!gpio_get(buttons[0].pin)){
    buttons[0].pressed = true;
  }
  if(buttons[0].pressed){
    shouldSwitch = !shouldSwitch;
    printf("%s\n", shouldSwitch ? "1 lima 2 moraito" : "1 moraito 2 lima");
    buttons[0].pressed = false;
  }

  //BUTTON2
  while(!gpio_get(buttons[1].pin)){
    buttons[1].pressed = true;
  }
  if(buttons[1].pressed){
    shouldSwitch = !shouldSwitch;
    printf("%s\n", shouldSwitch ? "1 lima 2 moraito" : "1 moraito 2 lima");
    buttons[1].pressed = false;
  }
}

int main() {

    setup();

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    char str[12];

    ws2812_program_init(pio, sm, offset, LEDS_PIN, 800000, false);

    while (1) {
        gpio_put(LED_PIN, !gpio_get(BUTTON1_PIN) || !gpio_get(BUTTON2_PIN));
        
        handleEncoder();

        handleButtons();

        uint encoder = counter > 255 ? abs(counter) % 255 : abs(counter);

        for (int i = 0; i < NUMPIXELS; i++) {
          put_pixel(shouldSwitch ? urgb_u32(encoder, 0x10, 0x05) : urgb_u32(0x05, encoder, 0x10));
          put_pixel(!shouldSwitch ? urgb_u32(encoder, 0x10, 0x05) : urgb_u32(0x05, encoder, 0x10));
        }

        gpio_put(SEG_1, 1);

        gpio_put(SEG_A, 0);
        gpio_put(SEG_B, 0);
        gpio_put(SEG_C, 0);

        sleep_ms(1);        
    }
}