#include "dmx_lib.h"


// TODO: it might be cleaner to use a struct for the config. And to pass this
// config in each function. This would be neater, we would have only "one"
// global var and this might allow multiple DMX port. Not really useful, but it
// would make the library more generic


/**************************************************************************
Function:
    int8_t dmx_init(struct* dmx_component DMX, PIO selectedPio, uint pin, uint16_t maxChannels)

Description:
    This function initializes the DMX lib that will be used to receive the
    DMX commands.
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a
    DMX component
    PIO pio - The pio that must be used for the program (pio0 or pio1)
    uint pin - The GPIO pin that will receive the DMX commands
    uint16_t maxChannels - Set the max number of channel per packet
    
Return:
    The SM number for the pio program or -1 if an error occurred
**************************************************************************/
int8_t dmx_init(struct dmx_component DMX, PIO selectedPio, uint pin, uint16_t maxChannels) {
    int8_t returnCode = -1; // By default we consider we'll have an error

    DMX.timeLinePastLow = 0;
    DMX.timeLinePastHigh = 0;
    DMX.currentChannel = 0;
    DMX.maxChannelCount = maxChannels;
    DMX.pin = pin;
    DMX.pio = selectedPio;

    gpio_init(DMX.pin); // init GPIO pin
    gpio_set_dir(DMX.pin, GPIO_IN); // set GPIO as an INPUT

    // use PIO0 core for our PIO program
    // find and return a location in instruction memory for our PIO program
    uint offset = pio_add_program(DMX.pio, &dmx_program);
    // find and return a free state machine
    DMX.sm = pio_claim_unused_sm(DMX.pio, true);

    if(DMX.sm != -1) {
        returnCode = 0; // no error

        // configure and run our PIO program
        dmx_program_init(DMX.pio, DMX.sm, offset, DMX.pin);

        // add an interrupt on the pin, so we can detect DMX packet start, DMX
        // connection and DMX disconnection
        gpio_set_irq_enabled_with_callback(DMX.pin,
            GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
            true,
            &dmx_pin_irq);
    }

    return returnCode;
}


/**************************************************************************
Function:
    int8_t dmx_get_slot(uint16_t* channelNumber, uint8_t* channelValue)

Description:
    This function will wait for the PIO program to push the DMX slot and 
    gives the slot's number and its value. 
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a 
    DMX component
    uint16_t* channelNumber - Pointer for the var that will receive the
    slot's number / index
    uint8_t* channelValue - Pointer for the var that will receive the value
    of the current slot / channel

Remark:
    This is a blocking function !
    Channel `0` is the Start Code
    
Return:
    0 - no error, slot received
    1 - no slot received (not really an error)
    else an error occurred (TODO: create error codes and describe them)
**************************************************************************/
int8_t dmx_get_slot(struct dmx_component DMX, uint16_t* channelNumber, uint8_t* channelValue) {
    uint8_t currentChannelValue = dmx_program_get_slot(DMX.pio, DMX.sm);

    *channelNumber = DMX.currentChannel;
    *channelValue = currentChannelValue;

    if(DMX.currentChannel >= DMX.maxChannelCount)
        dmx_reset_slot_index(DMX);
    else
        DMX.currentChannel++;

    return 0;
}


/**************************************************************************
Function:
    void dmx_reset_slot_index(void)

Description:
    This function resets the index of the current slot (used when we receive
    a new packet)
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a 
    DMX component
    
Return:
    None
**************************************************************************/
void dmx_reset_slot_index(struct dmx_component DMX) {
    // this function should be called when a `Space for Break` is received, in
    // other words, when the pin is low for more than 88μs

    DMX.currentChannel = 0;

    return;
}


/**************************************************************************
Function:
    void dmx_pin_irq(uint gpio, uint32_t events)

Description:
    This function will be called when a change (interrupt) is detected on the
    DMX pin. It will be used to detect a start of packet / a DMX connection, a
    DMX disconnection...
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a 
    DMX component
    uint gpio - The GPIO that triggered the interrupt
    uint32_t events - The events that triggered the interrupt
    
Return:
    None
**************************************************************************/
void dmx_pin_irq(struct dmx_component DMX, uint gpio, uint32_t events) {

    // quick check if the interrupts was really triggered by the DMX pin
    if(gpio == DMX.pin) {
        // printf("[Debug] %x\n", events);
        // TODO: fix event "c" (falling and rising edge at the same time)

        if((events & GPIO_IRQ_EDGE_FALL) > 0) {
            // falling edge detected -> line's low
            DMX.timeLinePastLow = time_us_64();
            gpio_acknowledge_irq(DMX.pin, GPIO_IRQ_EDGE_FALL);
        } else if ((events & GPIO_IRQ_EDGE_RISE) > 0) {
            // rising edge detected -> line's high
            DMX.timeLinePastHigh = time_us_64();
            gpio_acknowledge_irq(DMX.pin, GPIO_IRQ_EDGE_RISE);

            if((DMX.timeLinePastHigh - DMX.timeLinePastLow) > SFB_DURATION) {
                printf("\n[Debug] DMX Starts\n");
                dmx_reset_slot_index(DMX);
            }
        }
    }

    return;
}


/**************************************************************************
Function:
    uint64_t dmx_get_time_line_changed(bool toHigh)

Description:
    This function gives the time the DMX went High or Low
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a 
    DMX component
    bool toHigh - if true the function will return the time it went high, if
    false the function will return the time it went low
    
Return:
    Time in μs from the boot
**************************************************************************/
uint64_t dmx_get_time_line_changed(struct dmx_component DMX, bool toHigh) {
    uint64_t returnTime;
    
    if(toHigh) {
        returnTime = DMX.timeLinePastHigh;
    } else {
        returnTime = DMX.timeLinePastLow;
    }

    return returnTime;
}


/**************************************************************************
Function:
    enum DMX_State dmxStateMachine(enum DMX_State currentState)

Description:
    This function is used to manage the DMX without blocking the main loop (we 
    try to be as near as possible to a RT system)
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a 
    DMX component
    enum DMX_State currentState - The current state, so we can manage the next
    state based on the current one

Return:
    The new state
**************************************************************************/
enum DMX_State dmxStateMachine(struct dmx_component DMX, enum DMX_State currentState) {
    enum DMX_State nextState = currentState;

    switch (currentState)
    {
        case DMX_DISCONNECTED:
            break;
    
        case DMX_STARTED:
            if(dmxStateMachineDisconnect(DMX)) {
                nextState = DMX_DISCONNECTED;
                printf("[Debug] DMX Disconnected\n");
            }
            break;

        default:
            break;
    }

    return nextState;
}


/**************************************************************************
Function:
    bool dmxStateMachineDisconnect(void)

Description:
    This function checks if the DMX next state is Disconnected
    
Parameters:
    struct dmx_component DMX - the structure that will store all the vars for a 
    DMX component
    
Return:
    True (1) if next state is Disconnected, false (0) otherwise
**************************************************************************/
bool dmxStateMachineDisconnect(struct dmx_component DMX) {
    bool hasDisconnected = 0;

    // TODO: at disconnection might receive some odd slots like 0 and/or 255,
    // maybe it's possbile to fix this

    if(gpio_get(DMX.pin)) { // check if line's high
        uint64_t now = time_us_64();
        uint64_t timeHigh = dmx_get_time_line_changed(DMX, 1);

        if(now > timeHigh) {
            if((now - timeHigh) > (uint64_t) DISCO_DURATION) {
                hasDisconnected = 1;
            }
        }
    }

    return hasDisconnected;
}

