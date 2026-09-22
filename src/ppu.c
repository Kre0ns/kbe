#include "ppu.h"

#include "io.h"

void ppu_step(gb_t *gb, uint8_t cycles)
{
    gb->ppu.dots += cycles;

    if (gb->ppu.dots > 455)
    {
        gb->ppu.dots -= 456;

        gb->io[REG_LY] += 1;

        if (gb->io[REG_LY] > 153) gb->io[REG_LY] = 0;
    }
}