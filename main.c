#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "full_speed.pio.h"

#define LED_PIN PICO_DEFAULT_LED_PIN

int main () {

    // use PIO0 core for our PIO program
    PIO pio = pio0;

    // find and retrurn a location in instruction memory for our PIO program
    uint offset = pio_add_program(pio, &full_speed_program);

    // find and return a free state machine
    uint sm = pio_claim_unused_sm(pio, true);

    // configure and run our PIO program
    full_speed_program_init(pio, sm, offset, 22);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while(1) {
        gpio_put(LED_PIN, !gpio_get(LED_PIN));
        sleep_ms(500);
    }
}