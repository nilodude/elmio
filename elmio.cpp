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
#include "hardware/i2c.h"
#include "mcp23017.h"

const uint LED_PIN = 25; //GPIO25
const uint BUTTON1_PIN = 16; //GPIO16
const uint BUTTON2_PIN = 17; //GPIO17
const uint LEDS_PIN = 18; //GPIO15
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

uint currentDisplay = 0;
uint lastDisplay = 0;




static const bool MIRROR_INTERRUPTS = true; //save a gpio by mirroring interrupts across both banks
static const bool OPEN_DRAIN_INTERRUPT_ACTIVE = false;
static const bool POLARITY_INTERRUPT_ACTIVE_LOW = false;
static const int MCP_ALL_PINS_PULL_UP = 0xffff;
static const int MCP_ALL_PINS_COMPARE_TO_LAST = 0x0000;
static const int MCP_ALL_PINS_INTERRUPT_ENABLED = 0xffff;

const uint MCP_ALL_PINS_OUTPUT = 0x0000;
const uint I2C_GPIO_PIN_SDA = 6;
const uint I2C_GPIO_PIN_SLC = 7;

Mcp23017 mcp1(i2c0, 0x21);// MCP with A0,1,2 to GND



static inline void setup_output(Mcp23017 mcp) {
	int result;

	result = mcp.setup(false, true);
	result = mcp.set_io_direction(MCP_ALL_PINS_OUTPUT);
  printf("%s", result == -2 ? "error setup" : "setup ok");
}




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
    sleep_ms(2000);
    


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

   
}

static inline void handleEncoder(){
  currCLK = gpio_get(ENCODER_CLK);
        
  if (currCLK != lastCLK && currCLK){
    if(gpio_get(ENCODER_DT) != currCLK){
      counter--;
      counter = counter == -1 ? 3 : counter;
      currentDisplay = counter;
    }else{
      counter++;
      counter = counter == 4 ? 0 : counter;
      currentDisplay = counter;
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

    i2c_init(i2c0, 100000);
	  gpio_set_function(I2C_GPIO_PIN_SDA, GPIO_FUNC_I2C);
	  gpio_set_function(I2C_GPIO_PIN_SLC, GPIO_FUNC_I2C);
	  gpio_pull_up(I2C_GPIO_PIN_SDA);
	  gpio_pull_up(I2C_GPIO_PIN_SLC);

	  setup_output(mcp1);


    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    char str[12];

    ws2812_program_init(pio, sm, offset, LEDS_PIN, 800000, false);

    mcp1.set_all_output_bits(0xFFFF);

	  printf("Setting MCP(0x21) pin 4\n");

	  mcp1.set_output_bit_for_pin(4, false);

	  mcp1.flush_output();

    while (1) {
        gpio_put(LED_PIN, !gpio_get(BUTTON1_PIN) || !gpio_get(BUTTON2_PIN));
        
        handleEncoder();

        

        sleep_ms(1);        
    }
}     