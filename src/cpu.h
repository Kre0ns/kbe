#pragma once

#include "gb.h"

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

void cpu_init(gb_t *gb);

uint8_t cpu_step(gb_t *gb);