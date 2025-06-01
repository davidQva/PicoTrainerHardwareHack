#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "ssid.h"
#include "udp_client.h"
#include "hardware/watchdog.h"
#include <stdbool.h>
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"


// Definiera vilka pinnar vi använder
#define STEP_PIN 28
#define DIR_PIN 27
//#define BUTTON_PIN_1 4
//#define BUTTON_PIN_2 5
//#define LED 6
#define SWITCH_PIN 7
#define HALL_PIN 8

// Steppermotor pulse settings
#define STEP_DELAY_US 1700  // 1000 mikrosekunder = 1ms mellan steg

#define SERVER_IP "192.168.68.106"
#define SERVER_PORT 9988

volatile bool motor_fram = false;
volatile bool motor_bak = false;
volatile float current_position = 0.0f; // stepper motor position
volatile float resistance = 0.0f; // 0-100%
volatile float target_step = 30.0f;

int rotation_count = 0;
volatile bool oneSecInterupt = false;

void step_motor(bool direction) {
    // Sätt riktning
        gpio_put(DIR_PIN, direction);     
        
        gpio_put(STEP_PIN, 1);
        sleep_us(STEP_DELAY_US);
        gpio_put(STEP_PIN, 0);
        sleep_us(STEP_DELAY_US);
    
}

// maximum steps is 864 from home position   
void motor_loop() {
    while (true) {

        target_step = resistance*864 / 100.0f; // 864 steps = 360 degrees 
        
        if ((int)target_step != (int)current_position) {
            bool direction = (target_step > current_position) ? true : false;
            gpio_put(DIR_PIN, direction ? 1 : 0);
            step_motor(direction); // Stega framåt eller bakåt
            current_position += direction ? 1 : -1; // Lägg till eller subtrahera beroende på riktning
            //printf("Position: Current=%.2f, Target=%.2f, Resistance=%.2f\n", current_position, target_step, resistance);
        } else {
            sleep_ms(1); // ingen förändring, vila lite
        }
    }   
}

// Interrupt handler for rotation sensor
void sensor_interrupt_handler(uint gpio, uint32_t events) {
       rotation_count++; // Increment rotation count on each interrupt
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

bool repeating_timer_callback(struct repeating_timer *t) {
    oneSecInterupt = true; 
    return true; 
}

int main() { 

    stdio_init_all();  
    //Init WiFi// 
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
    //gpio_init(BUTTON_PIN_1);
    //gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    //gpio_pull_up(BUTTON_PIN_1); 

    //gpio_init(BUTTON_PIN_2);
    //gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    //gpio_pull_up(BUTTON_PIN_2); 

    //Pin is used to detect if the motor is in end position 
    gpio_init(SWITCH_PIN);
    gpio_set_dir(SWITCH_PIN, GPIO_IN); 
    gpio_pull_up(SWITCH_PIN); 

    // Separate core for motor control loop
    multicore_launch_core1(motor_loop);

    //Init rotationsensor and interrupt//

    // This pin is used for HAL sensor that is detecting the rotation of bike wheel 
    gpio_init(HALL_PIN);
    gpio_set_dir(HALL_PIN, GPIO_IN);
    
    //Using internal pulkl-up resistor dont work, hsyteresis is too low use 330k resistor instead
    //gpio_pull_up(HALL_PIN);

    // Interrup for HALL sensor, falling edge
    gpio_set_irq_enabled_with_callback(HALL_PIN, GPIO_IRQ_EDGE_FALL, true, &sensor_interrupt_handler);

    // Init repeating timer for sending RPM every second
    struct repeating_timer timer;
    add_repeating_timer_ms(1000, repeating_timer_callback, (void*)42, &timer);
   
    // Iff 0 do homing of stepper motor
    short init = 0;

    while (true) { 
             
       if (init == 0) {
        printf("Homing magnets\n");
         short back = 1;
           while(back) {
            step_motor(false); // Move backwards until the switch is pressed
            if (!gpio_get(SWITCH_PIN)) {
                back = 0; // Stop if the switch is pressed
            }
           }
           
           for (size_t i = 0; i < 30; i++)
           {
            step_motor(true); // Move away from the switch 
           }
                
           init = 1;
           current_position = 30;
           printf("Homing magnets complete\n");
        }
              
        // Kör framåt
        //if (!gpio_get(BUTTON_PIN_1)) {
        //step_motor(true);
        //}
        // Kör bakåt
        //if (!gpio_get(BUTTON_PIN_2) && gpio_get(SWITCH_PIN)) {
        //step_motor(false);
        //}
        if (oneSecInterupt) {
            oneSecInterupt = false;
            double rpm = rotation_count * 60;
            rotation_count = 0; // Reset count after sending
            char message[64];
            snprintf(message, sizeof(message), "RPM: %.2f", rpm / 2);
            send_message(message);
        }
    }
}

