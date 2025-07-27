#include "shell_cmds.h"
#include "compat.h"
#include <string.h>
#include <stdio.h>

int shell_cmd_help(char* out, int maxlen) {
    const char* help = "help clear reboot ls cat exit";
    int len = strlen(help);
    if (len > maxlen - 1) len = maxlen - 1;
    memcpy(out, help, len); out[len] = 0;
    return 0;
}

int shell_cmd_clear(void) {
    printf("\033[2J\033[H");
    return 0;
}

int shell_cmd_reboot(void) {
    extern void system_reboot(void);
    system_reboot();
    return 0;
}

int shell_cmd_exit(void) {
    return 1;
} 