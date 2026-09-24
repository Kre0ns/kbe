#include "joypad.h"

#include "io.h"

uint8_t joypad_read(gb_t *gb)
{
    bool select_dpad = !((gb->io[REG_JOYP] >> 4) & 0x01);
    bool select_buttons = !((gb->io[REG_JOYP] >> 5) & 0x01);

    uint8_t pressed = 0;
    if (select_dpad) pressed |= (gb->joypad.down << 3) | ( gb->joypad.up << 2) | ( gb->joypad.left << 1) | (gb->joypad.right);
    if (select_buttons) pressed |= (gb->joypad.start << 3) | ( gb->joypad.select << 2) | ( gb->joypad.b << 1) | (gb->joypad.a);

    return 0xC0 | (gb->io[REG_JOYP] & 0x30) | (~pressed & 0x0F);
}