#include "main.h"

#define LED_PIN             PICO_DEFAULT_LED_PIN
#define DMX_RECEIVE_PIN     9
#define DMX_STATE_LED_PIN   7

#define SFB_DURATION        88 // Space For Break duration - 88μs
#define DISCO_DURATION      1500000 // max MARKs duration - 1.5s

enum DMX_State dmxCurrentState = DMX_DISCONNECTED;

int main () {
    stdio_init_all();

    printf("[Debug] Program just started !\n");

    //TEMPO
    gpio_init(LED_PIN); // enables the pin for the LED
    gpio_set_dir(LED_PIN, GPIO_OUT); // set to OUTPUT
    //TEMPO END

    gpio_init(DMX_STATE_LED_PIN); // enables the pin for the LED
    gpio_set_dir(DMX_STATE_LED_PIN, GPIO_OUT); // set to OUTPUT
    // The LED is inverted: a 1 disables the led, a 0 enables the led 
    gpio_put(DMX_STATE_LED_PIN, 1);
    
    uint sm = dmx_init(pio0, DMX_RECEIVE_PIN);

    while(1) {
        dmxCurrentState = dmxStateMachine(dmxCurrentState);
        if(dmxCurrentState == DMX_DISCONNECTED) {
            gpio_put(DMX_STATE_LED_PIN, 1); // disconnected -> disable the led
        } else {
            gpio_put(DMX_STATE_LED_PIN, 0);// not disconnected -> enable the led
        }

        gpio_put(LED_PIN, gpio_get(DMX_RECEIVE_PIN));

        printf("[Debug] Wait for slot\n");
        uint8_t currentChannelValue = dmx_program_get_slot(pio0, sm);
        printf("[Debug] ==> slot value: %u\n", currentChannelValue);
        printf("[Debug] Slot received\n");
    }
}


enum DMX_State dmxStateMachine(enum DMX_State currentState) {
    enum DMX_State nextState;

    switch (currentState)
    {
        case DMX_DISCONNECTED:
            if(dmxStateMachineStarting()) {
                nextState = DMX_STARTS;
                printf("[Debug] DMX Starts\n");
            }
            break;
        
        case DMX_STARTS:
            if(dmxStateMachineDisconnect()) {
                nextState = DMX_DISCONNECTED;
                printf("[Debug] DMX Disconnected\n");
            } else if(dmxStateMachineStarted()) {
                nextState = DMX_STARTED;
                printf("[Debug] DMX Started\n");
            }
            break;

        case DMX_STARTED:
            if(dmxStateMachineDisconnect()) {
                nextState = DMX_DISCONNECTED;
                printf("[Debug] DMX Disconnected\n");
            } else if(dmxStateMachineStarting()) {
                nextState = DMX_STARTS;
                printf("[Debug] DMX Starts\n");
            }
            break;
        default:
            break;
    }

    return nextState;
}

bool dmxStateMachineStarting(void) {
    bool isStarting = 0;
    if(!gpio_get(DMX_RECEIVE_PIN)) { // check if line's low
        if(time_us_64() - dmx_get_time_line_changed(0) >= SFB_DURATION) {
            // SFB Detected
            isStarting = 1;
        }
    }
    return isStarting;
}

bool dmxStateMachineDisconnect(void) {
    bool hasDisconnected = 0;
    if(gpio_get(DMX_RECEIVE_PIN)) { // check if line's high
        if(dmx_get_time_line_changed(1) > dmx_get_time_line_changed(0)) {
            if(time_us_64() - dmx_get_time_line_changed(1) > DISCO_DURATION){
                hasDisconnected = 1;
            }
        }
    }
    return hasDisconnected;
}

bool dmxStateMachineStarted(void) {
    bool hasStarted = 0;
     // check if the interrupt has been triggered in the PIO program
    if(irq_is_enabled(7)) {
        hasStarted = 1;
        irq_clear(7); // clear the interrupts
    }
    return hasStarted;
}
