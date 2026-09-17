#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include "bus.h"

void cpu_init(gb_t *gb)
{
    gb->cpu.af = 0x01B0;
    gb->cpu.bc = 0x0013;
    gb->cpu.de = 0x00D8;
    gb->cpu.hl = 0x014D;
    gb->cpu.pc = 0x0100;
    gb->cpu.sp = 0xFFFE;
}

uint8_t cpu_step(gb_t *gb)
{
    uint8_t opcode = bus_read(gb, gb->cpu.pc++);

    switch (opcode)
    {
    default:
        fprintf(stderr, "Unimplemented opcode 0x%02X at 0x%04X\n", opcode, gb->cpu.pc - 1);
        exit(1);
    }
}