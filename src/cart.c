#include "cart.h"

#include <stdlib.h>
#include <stdio.h>

void cart_load(gb_t *gb, char *path)
{
    FILE *cart_file;
    long rom_size = -1L;

    cart_file = fopen(path, "rb");
    if (!cart_file)
    {
        perror("Failed to open cartridge");
        return;
    }
    
    if (fseek(cart_file, 0L, SEEK_END) != 0)
    {
        perror("Failed to seek cartridge");
        fclose(cart_file);
        return;
    }

    if ((rom_size = ftell(cart_file)) == -1L)
    {
        perror("Failed to get cartridge rom size");
        fclose(cart_file);
        return;
    }

    if (rom_size < 0x150)
    {
        fprintf(stderr, "Not a valid ROM: too small (%ld bytes)\n", rom_size);
        fclose(cart_file);
        return;
    }

    gb->cart.rom = malloc(rom_size * sizeof(uint8_t));
    if (gb->cart.rom == NULL)
    {
        perror("Failed to malloc for cartridge rom");
        fclose(cart_file);
        return;
    }

    rewind(cart_file);

    if (fread(gb->cart.rom, sizeof(uint8_t), rom_size, cart_file) != (size_t)rom_size)
    {
        perror("Failed to read cartridge");
        fclose(cart_file);
        free(gb->cart.rom);
        gb->cart.rom = NULL;
        return;
    }

    gb->cart.rom_size = rom_size;
    gb->cart.type = gb->cart.rom[0x147];

    fclose(cart_file);
}

uint8_t cart_read(gb_t *gb, uint16_t addr)
{
    if (addr <= 0x7FFF) return gb->cart.rom[addr];
    return 0xFF; // Currently no external RAM support
}

void cart_write(gb_t *gb, uint16_t addr, uint8_t val)
{
    (void)gb; (void)addr; (void)val;
    // Currently no external RAM support
}

void cart_free(gb_t *gb)
{
    free(gb->cart.rom);

    gb->cart.rom = NULL;
    gb->cart.rom_size = 0;
    gb->cart.type = 0;
}