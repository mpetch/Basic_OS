#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "stdio.h"
#include "io/keyboard.h"
#include "io/io.h"
#include "shell/shell.h"
#include "interrupts/idt.h"
#include "files/file.h"

void kernel_main(void) 
{
    //idt_init();
    filesInit();
    terminal_initialize();

    printf("Welcome to BasicOS!\nIt's very basic...\n\n");

    while (true) {
        query();
    }
}
