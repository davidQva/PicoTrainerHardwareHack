#pragma once

#include <stdbool.h>

void udp_client_init(void);
bool udp_client_connect(const char* ip, int port);
void udp_client_loop(void);
void send_message(const char* msg);
