#include "ppu.h"

#include "io.h"

static void render_scanline(gb_t *gb);

void ppu_step(gb_t *gb, uint8_t cycles)
{
    gb->ppu.dots += cycles;

    if (gb->ppu.dots > 455)
    {
        if (gb->io[REG_LY] < 144) render_scanline(gb);

        gb->ppu.dots -= 456;

        gb->io[REG_LY] += 1;

        if (gb->io[REG_LY] == 144) gb->io[REG_IF] |= 0x01;
        else if (gb->io[REG_LY] > 153) 
        {
            gb->io[REG_LY] = 0;
            gb->ppu.frame_ready = true;
        }  
    }
}

static void render_scanline(gb_t *gb)
{
    uint8_t y = gb->io[REG_LY];
    uint8_t bg_y = (gb->io[REG_SCY] + y);
    uint16_t base = gb->io[REG_LCDC] & 0x08 ? 0x1C00 : 0x1800;


    for (uint8_t x = 0; x < 160; x++)
    {
        uint8_t bg_x = (gb->io[REG_SCX] + x);

        uint8_t tile_idx = gb->vram[base + 32 * (bg_y / 8) + (bg_x /8)];

        uint16_t tile_offset = gb->io[REG_LCDC] & 0x10 ? tile_idx * 16 : 0x1000 + ((int8_t)tile_idx * 16);

        uint8_t low_byte = gb->vram[tile_offset + (bg_y % 8) * 2];
        uint8_t high_byte = gb->vram[tile_offset + (bg_y % 8) * 2 + 1];

        uint8_t shift = 7 - (bg_x % 8);

        uint8_t low_bit = (low_byte >> shift) & 0x01;
        uint8_t high_bit = (high_byte >> shift) & 0x01;

        uint8_t color_idx = (high_bit << 1) | low_bit;

        gb->ppu.frame_buffer[y][x] =  (gb->io[REG_BGP] >> (color_idx * 2)) & 0x03;
    }
}