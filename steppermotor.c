#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

// Definiera vilka pinnar vi använder
#define STEP_PIN 2
#define DIR_PIN 3

// Antal steg i varje riktning
#define STEPS_PER_MOVE 100

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

int main() {
    // Initiera Pico
    stdio_init_all();

    // Sätt upp pinnar som output
    gpio_init(STEP_PIN);
    gpio_set_dir(STEP_PIN, GPIO_OUT);

    gpio_init(DIR_PIN);
    gpio_set_dir(DIR_PIN, GPIO_OUT);

    while (true) {
        // Kör framåt
        step_motor(STEPS_PER_MOVE, true);
        sleep_ms(1000); // Vänta 0.5 sekund

        // Kör bakåt
        step_motor(STEPS_PER_MOVE, false);
        sleep_ms(1000); // Vänta 0.5 sekund
    }
}

