
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "lwip/udp.h"
#include <string.h>
#include <stdio.h>
#include "steppermotor.h"

#define LED_PIN 17
#define BUTTON_PIN 14

static struct udp_pcb* client_pcb;
static ip_addr_t remote_addr;
static u16_t remote_port;
int fram = 0, bak = 0;

static void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port) {
if (!p) return;

//absolute_time_t start = get_absolute_time();

char buffer[64] = {0};
memcpy(buffer, p->payload, p->len < 63 ? p->len : 63);
//printf("Mottaget: %s\n", buffer);



// Parsar state
sscanf(buffer, "fram=%d;bak=%d", &fram, &bak);

// Uppdatera globalt tillstånd
motor_fram = (fram == 1);
motor_bak = (bak == 1);
/*
// Hantera motorlogik
if (fram == 1 && bak == 0) {
printf("🔼 motor fram\n");
step_motor(true);
// gpio_put(MOTOR_PIN_FWD, true);
// gpio_put(MOTOR_PIN_BAK, false);
} else if (fram == 0 && bak == 1) {
printf("🔽 motor bak\n");
step_motor(false);
// gpio_put(MOTOR_PIN_FWD, false);
// gpio_put(MOTOR_PIN_BAK, true);
} else if (fram == 0 && bak == 0) {
printf("⛔️ motor stopp\n");
// gpio_put(MOTOR_PIN_FWD, false);
// gpio_put(MOTOR_PIN_BAK, false);
} else if (fram == 1 && bak == 1) {
printf("⚠️ Båda knappar nedtryckta – stopp för säkerhet\n");
// gpio_put(MOTOR_PIN_FWD, false);
// gpio_put(MOTOR_PIN_BAK, false);
}
*/
//absolute_time_t stop = get_absolute_time();
//printf("⚡️ Tid från mottagning till motorlogik: %lld us\n", absolute_time_diff_us(start, stop));

pbuf_free(p);
}


void send_message(const char* msg) {
    if (client_pcb && msg) {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
        if (!p) {
            printf("Failed to allocate packet buffer\n");
            return;
        }
        
        memcpy(p->payload, msg, strlen(msg));
        err_t err = udp_sendto(client_pcb, p, &remote_addr, remote_port);
        if (err != ERR_OK) {
            printf("Failed to send UDP packet: %d\n", err);
        }
        pbuf_free(p);
    }
}

static void button_callback(uint gpio, uint32_t events) {
    send_message("knapp_tryckt\n");
}

void udp_client_init(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

}

bool udp_client_connect(const char* ip, int port) {
    client_pcb = udp_new();
    if (!client_pcb) return false;

    if (!ip4addr_aton(ip, &remote_addr)) return false;
    remote_port = port;

    // Bind to any port (let lwIP choose)
    err_t err = udp_bind(client_pcb, IP_ADDR_ANY, 0);
    if (err != ERR_OK) {
        udp_remove(client_pcb);
        printf("Failed to bind UDP client: %d\n", err);
        return false;
    }

    // Set receive callback
    udp_recv(client_pcb, udp_recv_callback, NULL);
    
    // Send initial message to establish communication
    send_message("hello\n");
    
    return true;
}

void udp_client_loop(void) {
    while (true) {
        tight_loop_contents();
    }
}
