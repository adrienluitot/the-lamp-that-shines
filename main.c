#include "main.h"

#define DEBUG_LED_PIN       PICO_DEFAULT_LED_PIN
#define LM335_PIN           28
#define LED_PIN             21
#define FAN_ENABLE_PIN      20
#define DMX_RECEIVE_PIN     9
#define DMX_STATE_LED_PIN   7

#define TEMP_MEASURE_DELAY  1000000 // 1s

struct dmx_component DMX0;


int main () {
    /** Vars **/
    int16_t channelNumber = 0;
    uint8_t channelValue;
    uint8_t dmxCurrentSlots[6] = {0};
    uint8_t dmxSlots[6];
    bool packetReceived = false;

    enum DMX_Mode dmxMode = MODE_1;
    enum DMX_State dmxCurrentState = DMX_DISCONNECTED;
    enum DMX_State dmxLastState = DMX_DISCONNECTED;

    struct temp_config tempConfig;
    float heatSinkTemperature;

    /** Init Part **/
    stdio_init_all();
    adc_init(); // analogue digital converter
    //printf("[Debug] The program just started!\n");
    init_pins();

    temp_init(&tempConfig, LM335_PIN, TEMP_MEASURE_DELAY, 33.0f, 45.0f);
    heatSinkTemperature = temp_update_temperature(&tempConfig);


    /** DMX Part **/
    uint8_t errorDmxInit = dmx_init(&DMX0, pio0, DMX_RECEIVE_PIN, 6);

    // add an interrupt on the pin, so we can detect DMX packet start, DMX
    // connection and DMX disconnection
    gpio_set_irq_enabled_with_callback(DMX0.pin,
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_irq_dmx);

    while(1) {
        heatSinkTemperature = temp_update_temperature(&tempConfig);
        temp_update_machine_state(&tempConfig);

        if(tempConfig.temperatureState == TEMP_FAN) {
            gpio_put(FAN_ENABLE_PIN, 1);
        } else if(tempConfig.temperatureState == TEMP_SWITCH_OFF) {
            gpio_put(FAN_ENABLE_PIN, 1);
            // TODO : Switch off LED
        } else if(tempConfig.temperatureState == TEMP_COLD) {
            gpio_put(FAN_ENABLE_PIN, 0);
        }

        if(!dmx_get_slot(&DMX0, &channelNumber, &channelValue)) {
            if(channelNumber != 0) {
                dmxSlots[channelNumber-1] = channelValue;
            }
        }
        
        if(packetReceived && channelNumber == 0) {
            dmxCurrentState = DMX_STARTED;
            //printf("[Debug] DMX Started\n");
            packetReceived = false;
        } else if (!packetReceived && channelNumber == DMX0.maxChannelCount) {
            if(!utils_same_tables(dmxSlots, dmxCurrentSlots, 6)) {
                utils_copy_tables(dmxSlots, dmxCurrentSlots, 6);

                printf("\nSLOTS:\n");
                for(int8_t i = 0; i < 6; i++) {
                    printf("{%d}: %u\n", i+1, dmxCurrentSlots[i]);
                }

                if(dmxMode == MODE_1) {
                    pwm_set_gpio_level(LED_PIN, dmxCurrentSlots[0]*256);
                } else if(dmxMode == MODE_2) {
                    //pwm_set_gpio_level(LED_PIN, dmxCurrentSlots[0]*dmxCurrentSlots[1]);
                    pwm_set_gpio_level(LED_PIN, 65280-(dmxCurrentSlots[0]*128 + dmxCurrentSlots[1]*128));
                }
            }
            //printf("[Debug] Full packet received (%d/%d)\n", channelNumber, DMX0.maxChannelCount);
            packetReceived = true;
        }

        dmxCurrentState = dmxStateMachine(&DMX0, dmxCurrentState);
        dmxLastState = dmxCurrentState;

        // when disconnected -> disable the led (1 => disabled)
        gpio_put(DMX_STATE_LED_PIN, (dmxCurrentState == DMX_DISCONNECTED));
        if(dmxCurrentState == DMX_DISCONNECTED){
            utils_reset_table(dmxSlots, 6);
            utils_reset_table(dmxCurrentSlots, 6);
        }
    }
}

void init_pins(void) {
    gpio_init(LED_PIN); // enables the pin for the LED
    gpio_set_dir(LED_PIN, GPIO_OUT); // set to OUTPUT

    gpio_init(FAN_ENABLE_PIN);
    gpio_set_dir(FAN_ENABLE_PIN, GPIO_OUT);

    //TODO: TMP
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint pwmSliceNum = pwm_gpio_to_slice_num(LED_PIN);
    pwm_config pwmConfig = pwm_get_default_config();
    pwm_config_set_clkdiv(&pwmConfig, 4.f);
    pwm_init(pwmSliceNum, &pwmConfig, true);
    //TMP

    gpio_init(DMX_STATE_LED_PIN); // enables the pin for the LED
    gpio_set_dir(DMX_STATE_LED_PIN, GPIO_OUT); // set to OUTPUT
    // The LED is inverted: a 1 disables the led, a 0 enables the led 
    gpio_put(DMX_STATE_LED_PIN, 1);

    return;
}

void gpio_irq_dmx(uint gpio, uint32_t events) {
    dmx_pin_irq_handler(&DMX0, gpio, events);
}



