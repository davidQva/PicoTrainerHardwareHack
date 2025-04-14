#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

// Definiera vilka pinnar vi använder
#define STEP_PIN 2
#define DIR_PIN 3
#define BUTTON_PIN_1 4
#define BUTTON_PIN_2 5
#define LED 6
#define SWITCH_PIN 7

// Antal steg i varje riktning
#define STEPS_PER_MOVE 10

// Hastighet (ju högre delay, desto långsammare rörelse)
#define STEP_DELAY_US 1700  // 1000 mikrosekunder = 1ms mellan steg

void step_motor(int steps, bool direction) {
    // Sätt riktning
    gpio_put(DIR_PIN, direction);

    // Gå 'steps' antal steg
    for (int i = 0; i < steps; i++) {
        gpio_put(STEP_PIN, 1);
        sleep_us(STEP_DELAY_US);
        gpio_put(STEP_PIN, 0);
        sleep_us(STEP_DELAY_US);
    }
}

void light_led() {
    gpio_put(LED, 1); // Tänd LED
    sleep_ms(500); // Vänta 1 sekund
    gpio_put(LED, 0); // Släck LED
}

int main() {
    // Initiera Pico
    stdio_init_all();

    // Sätt upp pinnar som output
    gpio_init(STEP_PIN);
    gpio_set_dir(STEP_PIN, GPIO_OUT);

    gpio_init(DIR_PIN);
    gpio_set_dir(DIR_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN_1);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1); // Aktivera pull-up motstånd

    gpio_init(BUTTON_PIN_2);
    gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_2); // Aktivera pull-up motstånd

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT); // Sätt LED som output

    gpio_init(SWITCH_PIN);
    gpio_set_dir(SWITCH_PIN, GPIO_IN); // Sätt switch som input
    gpio_pull_up(SWITCH_PIN); // Aktivera pull-up motstånd

    while (true) {
        // Kör framåt
        if (!gpio_get(BUTTON_PIN_1)) {
            step_motor(STEPS_PER_MOVE, true);
        }
        //step_motor(STEPS_PER_MOVE, true);
        //sleep_ms(1000); // Vänta 0.5 sekund

        // Kör bakåt
        if (!gpio_get(BUTTON_PIN_2)) {
            step_motor(STEPS_PER_MOVE, false);
        }
        //step_motor(STEPS_PER_MOVE, false);
        //sleep_ms(1000); // Vänta 0.5 sekund

        if (!gpio_get(SWITCH_PIN)) {
            light_led(); // Tänd LED
        }
    }
}

