#include "idt.h"
#include <stdbool.h>

#define IDT_MAX_DESCRIPTORS 256

typedef struct {
    u16    isr_low;      // The lower 16 bits of the ISR's address
	u16    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	u8     reserved;     // Set to zero
	u8     attributes;   // Type and attributes; see the IDT page
	u16    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idtEntry;

struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) idtr;


void exception_handler(int vector_number, uint32_t error_code) {
    const char* exception_names[] = {
        "Divide Error", "Debug Exception", "Non-Maskable Interrupt",
        "Breakpoint", "Overflow", "Bound Range Exceeded", "Invalid Opcode",
        "Device Not Available", "Double Fault", "Coprocessor Segment Overrun",
        "Invalid TSS", "Segment Not Present", "Stack-Segment Fault",
        "General Protection Fault", "Page Fault", "(Reserved)",
        "x87 Floating-Point Exception", "Alignment Check",
        "Machine Check", "SIMD Floating-Point Exception"
    };

    if (vector_number < 20) {
        printf("Error: \"%s\" (Vector: %d, Error Code: 0x%x)\n",
               exception_names[vector_number], vector_number, error_code);
    } else {
        printf("Exception: Reserved or Unknown (Vector: %d, Error Code: 0x%08X)\n",
               vector_number, error_code);
    }
    asm __volatile__("cli; hlt");
}

__attribute__((aligned(16)))
idtEntry idt[IDT_MAX_DESCRIPTORS];

void idt_set_descriptor(u8 vector, void* isr, u8 flags) {
    idtEntry* descriptor = &idt[vector];

    descriptor->isr_low        = (u32)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (u32)isr >> 16;
    descriptor->reserved       = 0;
}

bool vectors[IDT_MAX_DESCRIPTORS];

extern void* isr_stub_table[];


#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1


void sendEOI(unsigned char irq) {
    if (irq > 7) {
        // Send EOI to the slave PIC
        outb(0xA0, 0x20);
    }
    // Send EOI to the master PIC
    outb(0x20, 0x20);
}

__attribute__((interrupt))
void timer_isr(struct interrupt_frame* frame) {

    sendEOI(0);
}


void init_idt() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (u16)sizeof(idtEntry) * IDT_MAX_DESCRIPTORS - 1;

    for (u8 vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    // Set the IRQ interrupts
    idt_set_descriptor(0x20, timer_isr, 0x8E);
    idt_set_descriptor(0x21, keyboard_isr, 0x8E);


    asm __volatile__ ("lidt %0" : : "m"(idtr)); // load the new IDT
    asm __volatile__ ("sti"); // set the interrupt flag
}

void remap_pic() {
    // Save current masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // Start initialization in cascade mode
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Remap IRQs: IRQ 0-7 to 0x20-0x27, IRQ 8-15 to 0x28-0x2F
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Cascade setup
    outb(PIC1_DATA, 0x04); // Tell Master PIC there is a slave at IRQ 2
    outb(PIC2_DATA, 0x02); // Tell Slave PIC its cascade identity

    // Set 8086/88 (MCS-80/85) mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restore saved masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}


// Initialize the IDT and PIC
void initPICIDT() {
    remap_pic();
    init_idt();
}