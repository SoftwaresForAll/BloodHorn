#ifndef BLOODHORN_ARP_H
#define BLOODHORN_ARP_H
#include <stdint.h>
#include "compat.h"
int arp_build_request(uint8_t* buf, const uint8_t* sender_mac, const uint8_t* sender_ip, const uint8_t* target_ip);
#endif 