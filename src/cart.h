#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "gb.h"

bool cart_load(gb_t *gb, char *path);

uint8_t cart_read(gb_t *gb, uint16_t addr);

void cart_write(gb_t *gb, uint16_t addr, uint8_t val);

void cart_free(gb_t *gb);