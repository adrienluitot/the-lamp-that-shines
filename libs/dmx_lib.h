#ifndef DMX_LIB
#define DMX_LIB

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "dmx.pio.h"
#include "hardware/pio.h"


#define SFB_DURATION        88 // Space For Break duration - 88μs
#define DISCO_DURATION      1000000 // max MARKs duration - 1s


/**************************************************************************
Description:
    These are the states for the non blocking state machine in the main function
    
    - DMX_DISCONNECTED is when no signal is detected for more than 1 second, the
    connection has probably been lost or never been connected
    - DMX_STARTS is when a Space for Break (low for more than 88μs) is received
    - DMX_STARTED is when we start to receive message
    
**************************************************************************/
enum DMX_State {
    DMX_DISCONNECTED=1, DMX_STARTED=2
};


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
int8_t dmx_init(PIO pio, uint pin, uint16_t maxChannels);


/**************************************************************************
Function:
    int8_t dmx_get_slot(uint16_t* channelNumber, uint8_t* channelValue)

Description:
    This function will wait for the PIO program to push the DMX slot and  gives
    the slot's number and its value. 
    
Parameters:
    uint16_t* channelNumber - Pointer for the var that will receive the slot's
    number / index
    uint8_t* channelValue - Pointer for the var that will receive the value of
    the current slot / channel

Remark:
    This is a blocking function !
    Channel `0` is the Start Code
    
Return:
    0 - no error, slot received
    1 - no slot received (not really an error)
    else an error occurred (TODO: create error codes and describe them)
**************************************************************************/
int8_t dmx_get_slot(uint16_t* channelNumber, uint8_t* channelValue);


/**************************************************************************
Function:
    void dmx_reset_slot_index(void)

Description:
    This function resets the index of the current slot (used when we receive a 
    new packet)
    
Parameters:
    None
    
Return:
    None
**************************************************************************/
void dmx_reset_slot_index(void);


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
void dmx_pin_irq(uint gpio, uint32_t events);


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
uint64_t dmx_get_time_line_changed(bool toHigh);


/**************************************************************************
Function:
    enum DMX_State dmxStateMachine(enum DMX_State currentState)

Description:
    This function is used to manage the DMX without blocking the main loop (we 
    try to be as near as possible to a RT system)
    
Parameters:
    enum DMX_State currentState - The current state, so we can manage the next
    state based on the current one

Return:
    The new state
**************************************************************************/
enum DMX_State dmxStateMachine(enum DMX_State currentState);


/**************************************************************************
Function:
    bool dmxStateMachineDisconnect(void)

Description:
    This function checks if the DMX next state is Disconnected
    
Parameters:
    None
    
Return:
    True (1) if next state is Disconnected, false (0) otherwise
**************************************************************************/
bool dmxStateMachineDisconnect(void);


#endif