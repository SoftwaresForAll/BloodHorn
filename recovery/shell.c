#include <stdint.h>
#include <string.h>
#include "shell.h"

#define MAX_CMD_LEN 256
#define MAX_ARGS 16

static char cmd_buffer[MAX_CMD_LEN];
static char* args[MAX_ARGS];
static int arg_count;

void shell_init(void) {
    printf("BloodHorn Rescue Shell v1.0\n");
    printf("Type 'help' for available commands\n");
}

void shell_parse_command(const char* input) {
    strcpy(cmd_buffer, input);
    arg_count = 0;
    char* token = strtok(cmd_buffer, " ");
    while (token && arg_count < MAX_ARGS) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
}

void shell_execute_command(void) {
    if (arg_count == 0) return;
    
    if (strcmp(args[0], "help") == 0) {
        printf("Available commands:\n");
        printf("  help     - Show this help\n");
        printf("  ls       - List files\n");
        printf("  cat <file> - Show file contents\n");
        printf("  reboot   - Reboot system\n");
        printf("  clear    - Clear screen\n");
    } else if (strcmp(args[0], "ls") == 0) {
        printf("Filesystem not mounted\n");
    } else if (strcmp(args[0], "cat") == 0) {
        if (arg_count > 1) {
            printf("File '%s' not found\n", args[1]);
        } else {
            printf("Usage: cat <filename>\n");
        }
    } else if (strcmp(args[0], "reboot") == 0) {
        printf("Rebooting...\n");
        // Call reboot function
    } else if (strcmp(args[0], "clear") == 0) {
        printf("\033[2J\033[H");
    } else {
        printf("Unknown command: %s\n", args[0]);
    }
}

void shell_run(void) {
    char input[MAX_CMD_LEN];
    while (1) {
        printf("bloodhorn> ");
        fgets(input, MAX_CMD_LEN, stdin);
        input[strlen(input) - 1] = 0;
        if (strlen(input) > 0) {
            shell_parse_command(input);
            shell_execute_command();
        }
    }
} 

EFI_STATUS shell_start(void) {
    shell_init();
    shell_run();
    return EFI_SUCCESS;
} 