#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    union
    {
        struct
        {
            uint8_t f;
            uint8_t a;
        };
        
        uint16_t af;
    };

    union
    {
        struct
        {
            uint8_t c;
            uint8_t b;
        };
        
        uint16_t bc;
    };

    union
    {
        struct
        {
            uint8_t e;
            uint8_t d;
        };
        
        uint16_t de;
    };

    union
    {
        struct
        {
            uint8_t l;
            uint8_t h;
        };
        
        uint16_t hl;
    };

    uint16_t sp;
    uint16_t pc;

    bool ime;
    bool halted;
    
} cpu_t;

typedef struct
{
    uint8_t *rom;
    size_t rom_size;

    uint8_t type;
} cart_t;

typedef struct
{
    cart_t cart;
    cpu_t cpu;
    
    uint8_t wram[0x2000];
    uint8_t vram[0x2000];
    uint8_t oam[0xA0];
    uint8_t hram[0x7F];
    uint8_t io[0x80];
    uint8_t ie;
} gb_t;

