#include <stdint.h>
#include "compat.h"

// BIOS-specific includes and code here

// BIOS entry point
void _start(void) {
    // BIOS initialization code here
    
    // Call BIOS-specific main function
    bios_main();
    
    // Halt if we return
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// BIOS main function
void bios_main(void) {
    // BIOS-specific initialization and main loop
    
    // Example: Clear screen (text mode)
    clear_screen();
    
    // Print welcome message
    print_string("BloodHorn BIOS Bootloader\n");
    print_string("Version 1.0\n\n");
    
    // TODO: Add BIOS-specific boot menu and functionality
    
    // Halt if we get here
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// Simple screen functions for BIOS
void clear_screen(void) {
    // Clear screen using BIOS interrupt
    __asm__ volatile (
        "mov $0x0600, %%ax\n"  // AH=06h (scroll up), AL=00h (full screen)
        "mov $0x07, %%bh\n"    // BH=07h (attribute)
        "xor %%cx, %%cx\n"     // CH=0, CL=0 (top-left corner)
        "mov $0x184f, %%dx\n"  // DH=18h, DL=4Fh (bottom-right corner)
        "int $0x10"             // Call BIOS video services
        : : : "ax", "bx", "cx", "dx"
    );
    
    // Move cursor to top-left
    __asm__ volatile (
        "mov $0x0200, %%ax\n"  // AH=02h (set cursor position)
        "xor %%bx, %%bx\n"     // BH=0 (page number)
        "xor %%dx, %%dx\n"     // DH=0, DL=0 (row, column)
        "int $0x10"             // Call BIOS video services
        : : : "ax", "bx", "dx"
    );
}

void print_char(char c) {
    // Print character using BIOS interrupt
    __asm__ volatile (
        "mov $0x0e, %%ah\n"    // AH=0Eh (teletype output)
        "int $0x10"             // Call BIOS video services
        : : "a" (0x0e00 | (c & 0xff)), "b" (0x0007)
    );
}

void print_string(const char *str) {
    while (*str) {
        print_char(*str++);
    }
}
