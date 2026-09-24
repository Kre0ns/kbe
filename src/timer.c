#include "timer.h"

#include "io.h"

void timer_step(gb_t *gb, uint8_t cycles)
{
    gb->timers.div_counter += cycles;

    if (gb->timers.div_counter > 255)
    {
        gb->timers.div_counter -= 256;

        gb->io[REG_DIV] += 1;
    }
}