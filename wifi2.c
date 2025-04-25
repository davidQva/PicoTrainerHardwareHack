#include <stdbool.h>
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "ssid.h"
#include "tcp_client2.h"
#include "hardware/watchdog.h"

//#define SERVER_IP "192.168.68.106"
//#define SERVER_PORT 9988

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
/*
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

    udp_client_loop(); // Håller igång kommunikationen
    return 0;
}
    */