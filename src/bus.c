#include "bus.h"

#include "cart.h"

uint8_t bus_read(gb_t *gb, uint16_t addr)
{
    if (addr <= 0x7FFF) return cart_read(gb, addr);
    else if (addr <= 0x9FFF) return gb->vram[addr - 0x8000];
    else if (addr <= 0xBFFF) return cart_read(gb, addr);
    else if (addr <= 0xDFFF) return gb->wram[addr - 0xC000];
    else if (addr <= 0xFDFF) return gb->wram[addr - 0xE000];
    else if (addr <= 0xFE9F) return gb->oam[addr - 0xFE00];
    else if (addr <= 0xFEFF) return 0xFF;
    else if (addr <= 0xFF7F) return gb->io[addr - 0xFF00];
    else if (addr <= 0xFFFE) return gb->hram[addr - 0xFF80];
    else return gb->ie;
}

void bus_write(gb_t *gb, uint16_t addr, uint8_t val)
{
    if (addr <= 0x7FFF) cart_write(gb, addr, val);
    else if (addr <= 0x9FFF) gb->vram[addr - 0x8000] = val;
    else if (addr <= 0xBFFF) cart_write(gb, addr, val);
    else if (addr <= 0xDFFF) gb->wram[addr - 0xC000] = val;
    else if (addr <= 0xFDFF) gb->wram[addr - 0xE000] = val;
    else if (addr <= 0xFE9F) gb->oam[addr - 0xFE00] = val;
    else if (addr <= 0xFEFF) { /*UNUSABLE*/ }
    else if (addr <= 0xFF7F) gb->io[addr - 0xFF00] = val;
    else if (addr <= 0xFFFE) gb->hram[addr - 0xFF80] = val;
    else gb->ie = val;
}