#include <stdio.h>

#include "gb.h"
#include "cart.h"
#include "cpu.h"

int main(int argc, char *argv[])
{
    (void)argc;

    gb_t gb = {0};
    gb_init(&gb);

    cart_load(&gb, argv[1]);

    printf("Title: %s\n", (char*)(gb.cart.rom + 0x134));

    cpu_step(&gb);

    cart_free(&gb);


    return 0;
}