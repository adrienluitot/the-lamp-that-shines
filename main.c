#include "main.h"

#define LED_PIN             PICO_DEFAULT_LED_PIN
#define DMX_RECEIVE_PIN     9
#define DMX_STATE_LED_PIN   7


int main () {
    stdio_init_all();

    printf("[Debug] The program just started!\n");

    init_pins(void);
    
    uint sm = dmx_init(pio0, DMX_RECEIVE_PIN, 6);

    int16_t channelNumber;
    uint8_t channelValue;

    enum DMX_State dmxCurrentState = DMX_DISCONNECTED;
    enum DMX_State dmxLastState = DMX_DISCONNECTED;

    while(1) {

        if(!pio_sm_is_rx_fifo_empty(pio0, sm)) {
            dmx_get_slot(&channelNumber, &channelValue);

            printf("[Debug] Slot [%u]: %u\n", channelNumber, channelValue);
            if(channelNumber == 0) {
                dmxCurrentState = DMX_STARTED;
                printf("[Debug] DMX Started\n");
            }
        }

        // TODO: save the packet/slots in a table to use them for real ^^ 

        dmxCurrentState = dmxStateMachine(dmxCurrentState);
        // when disconnected -> disable the led (1 => disabled)
        gpio_put(DMX_STATE_LED_PIN, (dmxCurrentState == DMX_DISCONNECTED));
        dmxLastState = dmxCurrentState;
    }
}

void init_pins(void) {
    gpio_init(LED_PIN); // enables the pin for the LED
    gpio_set_dir(LED_PIN, GPIO_OUT); // set to OUTPUT

    gpio_init(DMX_STATE_LED_PIN); // enables the pin for the LED
    gpio_set_dir(DMX_STATE_LED_PIN, GPIO_OUT); // set to OUTPUT
    // The LED is inverted: a 1 disables the led, a 0 enables the led 
    gpio_put(DMX_STATE_LED_PIN, 1);

    return;
}

