#include "gb.h"

#include <string.h>
#include "cpu.h"

void gb_init(gb_t *gb)
{
    memset(gb, 0, sizeof(*gb));
    cpu_init(gb);
}