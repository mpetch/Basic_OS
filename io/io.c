/*
    Functions for sending data through ports
*/

#include "io.h"

// Inline implementation of port I/O functions
// These functions provide direct hardware port access

// Read a byte from a specified I/O port
u8 inb(u16 port) {
    u8 result;
    
    // Inline assembly to read from port
    // "=a" (result) - output operand, storing result in eax
    // "d" (port) - input operand, port number in edx
    __asm__ volatile (
        "in %%dx, %%al"  // x86 instruction to read byte from port
        : "=a" (result)  // Output
        : "d" (port)     // Input
    );
    
    return result;
}

// Write a byte to a specified I/O port
void outb(u16 port, u8 data) {
    // Inline assembly to write to port
    // "a" (data) - input operand, value to write in eax
    // "d" (port) - input operand, port number in edx
    __asm__ volatile (
        "out %%al, %%dx"  // x86 instruction to write byte to port
        :                 // No output
        : "a" (data),     // Input value
          "d" (port)      // Port number
    );
}

// Read a 16-bit word from a port
u16 inw(u16 port) {
    u16 result;
    
    __asm__ volatile (
        "in %%dx, %%ax"  // Read 16-bit word from port
        : "=a" (result)
        : "d" (port)
    );
    
    return result;
}

// Write a 16-bit word to a port
void outw(u16 port, u16 data) {
    __asm__ volatile (
        "out %%ax, %%dx"  // Write 16-bit word to port
        :
        : "a" (data),
          "d" (port)
    );
}