#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "ssid.h"
#include "tcp_client2.h"
#include "hardware/watchdog.h"
#include <stdbool.h>
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"


// Definiera vilka pinnar vi använder
#define STEP_PIN 2
#define DIR_PIN 3
#define BUTTON_PIN_1 4
#define BUTTON_PIN_2 5
#define LED 6
#define SWITCH_PIN 7
#define HALL_PIN 8

// Antal steg i varje riktning
#define STEPS_PER_MOVE 10

// Hastighet (ju högre delay, desto långsammare rörelse)
#define STEP_DELAY_US 1700  // 1000 mikrosekunder = 1ms mellan steg

#define SERVER_IP "192.168.68.106"
#define SERVER_PORT 9988

volatile bool motor_fram = false;
volatile bool motor_bak = false;


void step_motor(bool direction) {
    // Sätt riktning
        gpio_put(DIR_PIN, direction);     
        
        gpio_put(STEP_PIN, 1);
        sleep_us(STEP_DELAY_US);
        gpio_put(STEP_PIN, 0);
        sleep_us(STEP_DELAY_US);
    
}

void motor_loop() {
    while (true) {
        if (motor_fram && !motor_bak) {
            step_motor(true);  // fram
        } else if (motor_bak && !motor_fram) {
            step_motor(false); // bak
        } else {
            sleep_ms(10); // Stillestånd
        }
    }
}

void light_led() {
    gpio_put(LED, 1); // Tänd LED
    //sleep_ms(1000); // Vänta 1 sekund
    gpio_put(LED, 0); // Släck LED
}

// Define the window size for RPM calculation (in milliseconds)
const uint64_t WINDOW_SIZE_MS = 5000;

// Debounce time in microseconds
const uint64_t DEBOUNCE_TIME_US = 60000; // 50ms debounce

// Max number of rotations expected in window (e.g. 200 RPM => ~17 in 5s)
const int MAX_ROTATIONS = 100;

uint64_t rotation_timestamps[100]; // Use a fixed size array
int rotation_count = 0;

volatile bool rotation_detected = false;
volatile uint64_t last_interrupt_time = 0;

// Interrupt handler for rotation sensor
void sensor_interrupt_handler(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64();

    if (current_time - last_interrupt_time > DEBOUNCE_TIME_US) {
        rotation_detected = true;
        last_interrupt_time = current_time;
    }
}

// Shift timestamps left to remove old entries
void cleanup_old_timestamps(uint64_t current_time_ms) {
    int new_count = 0;
    for (int i = 0; i < rotation_count; ++i) {
        if (current_time_ms - rotation_timestamps[i] <= WINDOW_SIZE_MS) {
            rotation_timestamps[new_count++] = rotation_timestamps[i];
        }
    }
    rotation_count = new_count;
}

bool wifi_connect(const char* ssid, const char* password) {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) {
        return false;
    }

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        return false;
    }

    return true;
}

int main() { 

    stdio_init_all();  
    
    if (!wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
        printf("Kunde inte ansluta till WiFi\n");
        printf("Startar om systemet...\n");
        cyw43_arch_deinit();
        watchdog_reboot(0, 0, 0);
        return 1;
    }

    udp_client_init();

    if (!udp_client_connect(SERVER_IP, SERVER_PORT)) {
        printf("Kunde inte initiera UDP-klient\n");
        return 1;
    }    

    //Steppermotor driver 
    gpio_init(STEP_PIN);
    gpio_set_dir(STEP_PIN, GPIO_OUT);
    gpio_init(DIR_PIN);
    gpio_set_dir(DIR_PIN, GPIO_OUT);


    //Test buttons for testing motor movement
    gpio_init(BUTTON_PIN_1);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1); 

    gpio_init(BUTTON_PIN_2);
    gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_2); 

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT); 

    // Initialize switch pin   
    // This pin is used to detect if the motor is in the home position
    gpio_init(SWITCH_PIN);
    gpio_set_dir(SWITCH_PIN, GPIO_IN); 
    gpio_pull_up(SWITCH_PIN); 

    // Initialze HALL sensor pin   
    // This pin is used to detect the rotation of the motor
    // and is connected to the HALL sensor
    gpio_init(HALL_PIN);
    gpio_set_dir(HALL_PIN, GPIO_IN);
    //Using internal pulkl-up resistor dont work, hsyteresis is too low use 330k resistor instead
    //gpio_pull_up(HALL_PIN);

    // Interrup for HALL sensor, falling edge
    gpio_set_irq_enabled_with_callback(HALL_PIN, GPIO_IRQ_EDGE_FALL, true, &sensor_interrupt_handler);

    short init = 0;

    while (true) { 
             
       if (init == 0) {
        printf("Homing magnets\n");
         short back = 1;
           while(back) {
            step_motor(false); // Drive backwards until the switch is pressed
            if (!gpio_get(SWITCH_PIN)) {
                back = 0; // Stop if the switch is pressed
            }
           }

           for (size_t i = 0; i < 30; i++)
           {
            step_motor(true); // Move forward a bit to avoid hitting the switch again
           }
           
                
            init = 1;
            printf("Homing magnets complete\n");
        }
              
        // Kör framåt
        if (!gpio_get(BUTTON_PIN_1)) {
            step_motor(true);
        }
        //step_motor(STEPS_PER_MOVE, true);
        //sleep_ms(1000); // Vänta 0.5 sekund

        // Kör bakåt
        if (!gpio_get(BUTTON_PIN_2) && gpio_get(SWITCH_PIN)) {
            step_motor(false);
        }
        //step_motor(STEPS_PER_MOVE, false);
        //sleep_ms(1000); // Vänta 0.5 sekund

        //Testa switch
        //if (!gpio_get(SWITCH_PIN)) {
        //    light_led(); // Tänd LED
        //}        

        // 
        if (rotation_detected) {
            rotation_detected = false;

            uint64_t current_time_ms = time_us_64() / 1000;

            // Save the timestamp of the rotation event if we have space
            if (rotation_count < MAX_ROTATIONS) {
                rotation_timestamps[rotation_count++] = current_time_ms;
            }

            // Remove old timestamps that are outside the window
            cleanup_old_timestamps(current_time_ms);

            // Calculate RPM if we have enough rotations
            if (rotation_count > 1) {
                double window_minutes = WINDOW_SIZE_MS / 60000.0;
                double rpm = rotation_count / window_minutes;

                printf("Current RPM: %.2f (based on %d pulses in %.2f seconds)\n",
                       rpm/2,
                       rotation_count,
                       WINDOW_SIZE_MS / 1000.0);
            }
        }

        multicore_launch_core1(motor_loop);

        //udp_client_loop();
    }
}

