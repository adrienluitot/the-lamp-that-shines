#include "dmx_lib.h"

PIO pio;
uint sm, dmxPin;
uint64_t timeLinePastLow, timeLinePastHigh;
uint16_t maxChannelCount, currentChannel;

// TODO: it might be cleaner to use a struct for the config. And to pass this
// config in each function. This would be neater, we would have only "one"
// global var and this might allow multiple DMX port. Not really useful, but it
// would make the library more generic


/**************************************************************************
Function:
    int8_t dmx_init(PIO pio, uint pin)

Description:
    This function initializes the DMX lib that will be used to receive the
    DMX commands.
    
Parameters:
    PIO pio - The pio that must be used for the program (pio0 or pio1)
    uint pin - The GPIO pin that will receive the DMX commands
    uint16_t maxChannels - Set the max number of channel per packet
    
Return:
    The SM number for the pio program or -1 if an error occurred
**************************************************************************/
int8_t dmx_init(PIO selectedPio, uint pin, uint16_t maxChannels) {
    int8_t returnCode = -1; // By default we consider we'll have an error

    timeLinePastLow = 0;
    timeLinePastHigh = 0;
    currentChannel = 0;

    dmxPin = pin;
    maxChannelCount = maxChannels;

    gpio_init(dmxPin); // init GPIO pin
    gpio_set_dir(dmxPin, GPIO_IN); // set GPIO as an INPUT

    // use PIO0 core for our PIO program
    pio = selectedPio;
    // find and return a location in instruction memory for our PIO program
    uint offset = pio_add_program(pio, &dmx_program);
    // find and return a free state machine
    sm = pio_claim_unused_sm(pio, true);

    if(sm != -1) {
        // we could get a State Machine for the PIO, we change the return code
        // to the state machine number 
        returnCode = sm;

        // configure and run our PIO program
        dmx_program_init(pio, sm, offset, dmxPin);

        // add an interrupt on the pin, so we can detect DMX packet start, DMX
        // connection and DMX disconnection
        gpio_set_irq_enabled_with_callback(dmxPin,
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
    uint16_t* channelNumber - Pointer for the var that will receive the
    slot's number / index
    uint8_t* channelValue - Pointer for the var that will receive the value
    of the current slot / channel

Parameters:
    This is a blocking function !
    Channel `0` is the Start Code
    
Return:
    0 - no error, slot received
    1 - no slot received (not really an error)
    else an error occurred (TODO: create error codes and describe them)
**************************************************************************/
int8_t dmx_get_slot(uint16_t* channelNumber, uint8_t* channelValue) {
    uint8_t currentChannelValue = dmx_program_get_slot(pio, sm);

    *channelNumber = currentChannel;
    *channelValue = currentChannelValue;

    if(currentChannel >= maxChannelCount)
        //currentChannel = 0;
        dmx_reset_slot_index();
    else
        currentChannel++;

    return 0;
}


/**************************************************************************
Function:
    void dmx_reset_slot_index(void)

Description:
    This function resets the index of the current slot (used when we receive
    a new packet)
    
Parameters:
    None
    
Return:
    None
**************************************************************************/
void dmx_reset_slot_index(void) {
    // this function should be called when a `Space for Break` is received, in
    // other words, when the pin is low for more than 88μs

    currentChannel = 0;

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
    uint gpio - The GPIO that triggered the interrupt
    uint32_t events - The events that triggered the interrupt
    
Return:
    None
**************************************************************************/
void dmx_pin_irq(uint gpio, uint32_t events) {

    // quick check if the interrupts was really triggered by the DMX pin
    if(gpio == dmxPin) {
        printf("[Debug] %x\n", events);

        if((events & GPIO_IRQ_EDGE_FALL) > 0) {
            // falling edge detected -> line's low
            timeLinePastLow = time_us_64();
            gpio_acknowledge_irq(dmxPin, GPIO_IRQ_EDGE_FALL);
        } else if ((events & GPIO_IRQ_EDGE_RISE) > 0) {
            // rising edge detected -> line's high
            timeLinePastHigh = time_us_64();
            gpio_acknowledge_irq(dmxPin, GPIO_IRQ_EDGE_RISE);
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
    bool toHigh - if true the function will return the time it went high, if
    false the function will return the time it went low
    
Return:
    Time in μs from the boot
**************************************************************************/
uint64_t dmx_get_time_line_changed(bool toHigh) {
    uint64_t returnTime;
    
    if(toHigh) {
        returnTime = timeLinePastHigh;
    } else {
        returnTime = timeLinePastLow;
    }

    return returnTime;
}
