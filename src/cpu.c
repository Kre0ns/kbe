#include "cpu.h"

void cpu_init(gb_t *gb)
{
    gb->cpu.af = 0x01B0;
    gb->cpu.bc = 0x0013;
    gb->cpu.de = 0x00D8;
    gb->cpu.hl = 0x014D;
    gb->cpu.pc = 0x0100;
    gb->cpu.sp = 0xFFFE;
}