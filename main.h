#include "pico/stdlib.h"
#include "hardware/irq.h"
#include <stdio.h>
#include "libs/dmx_lib.h"


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
    bool dmxStateMachineStarting(void)

Description:
    This function checks if the DMX next state is Starts
    
Parameters:
    None
    
Return:
    True (1) if next state is Starts, false (0) otherwise
**************************************************************************/
bool dmxStateMachineStarting(void);

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

/**************************************************************************
Function:
    bool dmxStateMachineStarted(void)

Description:
    This function checks if the DMX next state is Started
    
Parameters:
    None
    
Return:
    True (1) if next state is Started, false (0) otherwise
**************************************************************************/
bool dmxStateMachineStarted(void);