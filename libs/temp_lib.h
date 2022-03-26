#ifndef TEMPERATURE_LIB
#define TEMPERATURE_LIB

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "utils.h"

static const float VOLTAGE_CONVERT_FACTOR = 3.3f / (1 << 12); // 12 bits

#define TEMP_DOWN_LEEWAY 2

/**************************************************************************
Description:
    These are the states for the non blocking state machine of the temperatue in
    the main function
    
    - TEMP_COLD is when the temperature is cold enough
    - TEMP_FAN is when the temperature is high enough to switch on the fan
    - TEMP_SWITCH_OFF is when we the temperature is too high for the led
    
**************************************************************************/
enum TEMP_STATE {
    TEMP_COLD=1, TEMP_FAN=2, TEMP_SWITCH_OFF=3
};

/**************************************************************************
Description:
    This is the structure for a temp config
**************************************************************************/
struct temp_config
{
    uint pin, adcIndex;
    float fanTempThreshold, switchOffTempThreshold;
    uint64_t tempMeasureCounterCounter;
    uint32_t measureDelay;
    float voltageMeasure;
    float temperature;
    enum TEMP_STATE temperatureState;
};


/**************************************************************************
Function:
    bool temp_init( struct temp_config* config, uint pin, uint32_t measureDelay,
                    float fanTempThreshold, float switchOffTempThreshold)

Description:
    This function will initialize the temperature management lib.
    
Parameters:
    struct temp_config* config - the struct that will keep the config
    uint pin - the pin that reads the temperature
    uint32_t measureDelay - the delay between temp measurement
    float fanTempThreshold - the temperature threshold to enable the fan
    float switchOffTempThreshold - the temperature threshold to shut off the LED
    
Return:
    None
**************************************************************************/
void temp_init( struct temp_config* config, uint pin, uint32_t measureDelay,
                float fanTempThreshold, float switchOffTempThreshold);


/**************************************************************************
Function:
    int8_t temp_get_gpio_adc_input(uint gpio)

Description:
    This function returns the adc input index for the given GPIO
    
Parameters:
    uint gpio - Index of the gpio
    
Return:
    If the gpio is between 26 and 29 (included) the function will return the adc
    input index, other wise the gpio doesn't have a ADC HW module, so it will
    return -1 as an error
**************************************************************************/
int8_t temp_get_gpio_adc_input(uint gpio);


/**************************************************************************
Function:
    float temp_update_temperature(struct temp_config* config)

Description:
    This function update the temperature if the delay has passed. Returns also
    the current temperature.
    
Parameters:
    struct temp_config* config - config of the temp lib
    
Return:
    The current temperature or the last saved temperature
**************************************************************************/
float temp_update_temperature(struct temp_config* config);


/**************************************************************************
Function:
    enum TEMP_STATE temp_update_machine_state(struct temp_config* config)

Description:
    This function update the temperature if the delay has passed. Returns also
    the current temperature.
    
Parameters:
    struct temp_config* config - config of the temp lib
    
Return:
    The current temperature or the last saved temperature
**************************************************************************/
enum TEMP_STATE temp_update_machine_state(struct temp_config* config);


#endif