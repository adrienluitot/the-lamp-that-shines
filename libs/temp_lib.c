#include "temp_lib.h"

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
    float switchOffTempThreshold - the temperature threshold to switchoff the LED
    
Return:
    None
**************************************************************************/
void temp_init( struct temp_config* config, uint pin, uint32_t measureDelay,
                float fanTempThreshold, float switchOffTempThreshold) {
    
    config->pin = pin;
    config->measureDelay = measureDelay;
    config->fanTempThreshold = fanTempThreshold;
    config->switchOffTempThreshold = switchOffTempThreshold;
    config->tempMeasureCounterCounter = 0;
    config->voltageMeasure= 0;
    config->temperature = 0;
    config->adcIndex = temp_get_gpio_adc_input(pin);
    config->temperatureState = TEMP_COLD;

    // set the pin to ADC input pin
    adc_gpio_init(pin);

    return;
}


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
int8_t temp_get_gpio_adc_input(uint gpio) {
    int8_t returnCode = -1;

    if(gpio >= 26 && gpio <= 29) {
        returnCode = gpio - 26;
    }

    return returnCode;
}


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
float temp_update_temperature(struct temp_config* config) {
    if(utils_delay_us(&config->tempMeasureCounterCounter, config->measureDelay)){
        adc_select_input(config->adcIndex);
        config->voltageMeasure = adc_read()*VOLTAGE_CONVERT_FACTOR;
        config->temperature  = (config->voltageMeasure * 100) - 273.15;

        // printf("%f -> %f°C\n", config->voltageMeasure, config->temperature);
    }

    return config->temperature;
}


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
enum TEMP_STATE temp_update_machine_state(struct temp_config* config) {
    switch (config->temperatureState)
    {
        default:
        case TEMP_COLD:
            if(config->temperature >= config->switchOffTempThreshold)
                config->temperatureState = TEMP_SWITCH_OFF;
            else if(config->temperature >= config->fanTempThreshold)
                config->temperatureState = TEMP_FAN;
            break;
        
        case TEMP_FAN:
            if(config->temperature >= config->switchOffTempThreshold)
                config->temperatureState = TEMP_SWITCH_OFF;
            else if(config->temperature < (config->fanTempThreshold - TEMP_DOWN_LEEWAY))
                config->temperatureState = TEMP_COLD;
            break;

        case TEMP_SWITCH_OFF:
            if(config->temperature < (config->fanTempThreshold - TEMP_DOWN_LEEWAY))
                config->temperatureState = TEMP_SWITCH_OFF;
            else if(config->temperature < (config->switchOffTempThreshold - TEMP_DOWN_LEEWAY))
                config->temperatureState = TEMP_FAN;
    }

    return config->temperature;
}