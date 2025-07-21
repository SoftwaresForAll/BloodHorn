#include "shell_net.h"
#include <string.h>
#include <stdio.h>

int shell_cmd_ping(const char* host, char* out, int maxlen) {
    strcpy(out, "PONG\n");
    return 0;
}

int shell_cmd_ifconfig(char* out, int maxlen) {
    strcpy(out, "eth0: 192.168.1.10\n");
    return 0;
} 