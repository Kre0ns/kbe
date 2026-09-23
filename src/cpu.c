#include "cpu.h"

#include <stdio.h>
#include "bus.h"
#include "ops.h"
#include "io.h"

void push_u16(gb_t *gb, uint16_t val)
{
    bus_write(gb, --gb->cpu.sp, val >> 8);
    bus_write(gb, --gb->cpu.sp, val & 0x00FF);
}

uint16_t pop_u16(gb_t *gb)
{
    uint8_t lsb = bus_read(gb, gb->cpu.sp++);
    uint8_t msb = bus_read(gb, gb->cpu.sp++);
    return (uint16_t)(msb << 8) | lsb;
}


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
    if (gb->cpu.halted)
    {
        if (gb->ie & gb->io[REG_IF] & 0x1F) gb->cpu.halted = false;
        else return 4;
    }

    for (int i = 0; i < 5; i++)
    {
        if (gb->cpu.ime && (gb->ie & (1 << i)) && (gb->io[REG_IF] & (1 << i)))
        {
            gb->io[REG_IF] &= ~(1 << i);
            gb->cpu.ime = false;

            push_u16(gb, gb->cpu.pc);
            gb->cpu.pc = 0x40 + i * 8;
            
            return 20;
        }
    }

    bool enable_ime = false;
    uint8_t opcode = bus_read(gb, gb->cpu.pc++);

    enable_ime = gb->cpu.ime_pending;

    uint8_t cycles = cpu_execute(gb, opcode);

    if (enable_ime && gb->cpu.ime_pending)
    {
        gb->cpu.ime_pending = false;
        gb->cpu.ime = true;
    }

    return cycles;
}