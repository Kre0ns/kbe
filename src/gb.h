#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    uint8_t *rom;
    size_t rom_size;

} cart_t;

typedef struct
{
    cart_t cart;
    
    uint8_t wram[0x2000];
    uint8_t vram[0x2000];
    uint8_t oam[0xA0];
    uint8_t hram[0x7F];
    uint8_t io[0x80];
    uint8_t ie;
} gb_t;

