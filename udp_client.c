
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

#define MAX_STEPS 864

volatile float target_step_float = 0.0f;

static void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port) {
    if (!p) return;

    char buffer[64] = {0};
    memcpy(buffer, p->payload, p->len < 63 ? p->len : 63);

    int percent = 0;
    if (sscanf(buffer, "percent=%d", &percent) == 1) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        
        static int last_percent = -1;
        if (percent != last_percent) {
            last_percent = percent;
            resistance = percent;
            printf("Styrsignal: %d%%\n", percent);  }            
    }

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
