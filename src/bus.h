#pragma once
#include <stdint.h>
#include "gb.h"

uint8_t bus_read(gb_t *gb, uint16_t addr);

void bus_write(gb_t *gb, uint16_t addr, uint8_t val);