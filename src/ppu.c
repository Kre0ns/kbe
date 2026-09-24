#include "ppu.h"

#include <stdbool.h>
#include <string.h>

#include "io.h"

static void render_scanline(gb_t *gb);

void ppu_step(gb_t *gb, uint8_t cycles)
{
    bool lcd_on = gb->io[REG_LCDC] & 0x80;

    gb->ppu.dots += cycles;

    if (gb->ppu.dots <= 455) return;
    gb->ppu.dots -= 456;

    if (lcd_on && gb->ppu.line < 144) render_scanline(gb);

    gb->ppu.line++;

    if (lcd_on && gb->ppu.line == 144) gb->io[REG_IF] |= 0x01;

    if (gb->ppu.line > 153) 
    {
        gb->ppu.line = 0;

        if (!lcd_on) memset(gb->ppu.frame_buffer, 0, sizeof(gb->ppu.frame_buffer));

        gb->ppu.frame_ready = true;
    }

    gb->io[REG_LY] = lcd_on ? gb->ppu.line : 0;
}

static void render_scanline(gb_t *gb)
{
    uint8_t y = gb->ppu.line;

    uint8_t bg_color_idxs[160] = { 0 };

    if (gb->io[REG_LCDC] & 0x01)
    {
        uint8_t bg_y = (gb->io[REG_SCY] + y);
        uint16_t bg_base = gb->io[REG_LCDC] & 0x08 ? 0x1C00 : 0x1800;

        for (uint8_t x = 0; x < 160; x++)
        {
            uint8_t bg_x = (gb->io[REG_SCX] + x);

            uint8_t tile_idx = gb->vram[bg_base + 32 * (bg_y / 8) + (bg_x /8)];

            uint16_t tile_offset = gb->io[REG_LCDC] & 0x10 ? tile_idx * 16 : 0x1000 + ((int8_t)tile_idx * 16);

            uint8_t low_byte = gb->vram[tile_offset + (bg_y % 8) * 2];
            uint8_t high_byte = gb->vram[tile_offset + (bg_y % 8) * 2 + 1];

            uint8_t shift = 7 - (bg_x % 8);

            uint8_t low_bit = (low_byte >> shift) & 0x01;
            uint8_t high_bit = (high_byte >> shift) & 0x01;

            bg_color_idxs[x] = (high_bit << 1) | low_bit;

            gb->ppu.frame_buffer[y][x] =  (gb->io[REG_BGP] >> (bg_color_idxs[x] * 2)) & 0x03;
        }
    }
    else
    {
        for (uint8_t x = 0; x < 160; x++)
        {
            gb->ppu.frame_buffer[y][x] = gb->io[REG_BGP] & 0x03;
        }
    }

    if (gb->io[REG_LCDC] & 0x02)
    {
        uint8_t obj_height = (gb->io[REG_LCDC] & 0x04) ? 16 : 8;
        uint8_t obj_idxs[10] = { 0 };
        uint8_t obj_count = 0;

        for (uint8_t i = 0; i < 40 && obj_count < 10; i++)
        {
            uint8_t obj_y = gb->oam[i * 4] ;

            if (obj_y <= y + 16 && y + 16 < obj_y + obj_height)
            {
                obj_idxs[obj_count++] = i;
            }
        }

        for (uint8_t i = 1 ; i < obj_count; i++)
        {
            uint8_t key = obj_idxs[i];

            int8_t j = i - 1;

            while (j >= 0 && gb->oam[obj_idxs[j] * 4 + 1] > gb->oam[key * 4 + 1])
            {
                obj_idxs[j + 1] = obj_idxs[j];
                j--;
            }

            obj_idxs[j + 1] = key;
        }

        for (uint8_t x = 0; x < 160; x++)
        {
            for (uint8_t i = 0; i < obj_count; i++)
            {
                uint16_t oa_offset = obj_idxs[i] * 4;
                uint8_t oa_x = gb->oam[oa_offset + 1];

                if (oa_x <= x + 8 && x + 8 < oa_x + 8)
                {
                    uint8_t oa_flags = gb->oam[oa_offset + 3];

                    uint8_t tile_idx = (obj_height == 16) ? gb->oam[oa_offset + 2] & 0xFE : gb->oam[oa_offset + 2];

                    uint8_t pixel_y = y + 16 - gb->oam[oa_offset];
                    uint8_t pixel_x = x + 8 - oa_x;

                    if (oa_flags & 0x40) pixel_y = obj_height - 1 - pixel_y;
                    if (oa_flags & 0x20) pixel_x = 7 - pixel_x;

                    uint8_t low_byte = gb->vram[tile_idx * 16 + pixel_y * 2];
                    uint8_t high_byte = gb->vram[tile_idx * 16 + pixel_y * 2 + 1];

                    uint8_t shift = 7 - pixel_x;

                    uint8_t low_bit = (low_byte >> shift) & 0x01;
                    uint8_t high_bit = (high_byte >> shift) & 0x01;

                    uint8_t color_idx = (high_bit << 1) | low_bit;

                    if (color_idx == 0) continue;

                    if (oa_flags & 0x80 && bg_color_idxs[x] != 0) break;

                    uint8_t palette = gb->io[(oa_flags & 0x10) ? REG_OBP1 : REG_OBP0];

                    gb->ppu.frame_buffer[y][x] =  (palette >> (color_idx * 2)) & 0x03;

                    break;
                }
            }
        }
    }
}