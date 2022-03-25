#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "libs/dmx_lib.h"
#include "libs/utils.h"

enum DMX_Mode {
    MODE_1=1, MODE_2=2, MODE_3=3, MODE_4=4
};

/**************************************************************************
Function:
    void init_pins(void)

Description:
    This function inits the pins, must be launched in the main/setup function
    before the infinite loop
    
Parameters:
    None

Return:
    None
**************************************************************************/
void init_pins(void);


/**************************************************************************
Function:
    void gpio_irq_dmx(uint gpio, uint32_t events)


Description:
    This function will be called when a change (interrupt) is detected on the
    DMX pin. It will redirect the parameters to the real function that will
    handle the interrupt...
    
Parameters:
    uint gpio - The GPIO that triggered the interrupt
    uint32_t events - The events that triggered the interrupt

Return:
    None
**************************************************************************/
void gpio_irq_dmx(uint gpio, uint32_t events);

